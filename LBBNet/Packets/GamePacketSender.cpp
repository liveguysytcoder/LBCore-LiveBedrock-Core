#include "GamePacketSender.h"
#include "../../LBNet/Packets/FrameSetPacket.h"
#include "DataTypeHelper.h"
#include "Encryption.h"
#include "Compression.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <utility>

using namespace std;

void queueGamePacket(ClientState& state, const vector<uint8_t>& packetBody) {
    // [DEBUG] Raw plaintext game-packet bytes (packet ID + fields), before
    // batching/encryption.
    cout << "[DEBUG] Queued game packet body (" << packetBody.size() << " bytes): ";
    for (uint8_t b : packetBody) {
        cout << hex << setw(2) << setfill('0') << (int)b << " ";
    }
    cout << dec << "\n";

    // Every packet in a Bedrock game-packet batch is its own
    // varint(length) + body inside the shared batch buffer -- this is
    // unchanged from before. What changes is WHERE this goes: onto
    // state.pendingBatch, to be combined with whatever else gets queued
    // before the next flushGamePackets() call, instead of being
    // immediately wrapped, encrypted, and sent on its own.
    writeVarInt(state.pendingBatch, (uint32_t)packetBody.size());
    state.pendingBatch.insert(state.pendingBatch.end(), packetBody.begin(), packetBody.end());
}

void flushGamePackets(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Confirmed directly against gophertunnel's real Conn (conn.go):
    // WritePacket() only ever appends to an internal bufferedSend list --
    // it never sends anything itself. A separate Flush() (called
    // automatically after every incoming packet is handled, and also on a
    // periodic ~20th-of-a-second ticker) is what actually combines
    // EVERYTHING queued since the last flush into ONE compressed/encrypted
    // batch and writes it to the connection in a single call:
    // `conn.enc.Encode(toSend)`, passing the whole accumulated list at
    // once. This server was instead calling the equivalent of
    // WritePacket()+Flush() together for every single packet -- StartGame
    // and ItemRegistry, for instance, went out as two completely separate
    // encrypted RakNet messages instead of one combined batch the way a
    // real server sends them. That mismatch was invisible at the byte
    // level of either packet individually (which is why checking StartGame
    // and ItemRegistry's own content, however thoroughly, could never
    // catch it) -- it only shows up in how many separate sends happen and
    // when. queueGamePacket() now only buffers; this function is the
    // actual send, meant to be called once after a batch of related
    // packets have all been queued (see call sites: after StartGame +
    // ItemRegistry, after the ResourcePackClientResponse-triggered
    // PlayStatus + ResourcePacksInfo, after the RequestChunkRadius-
    // triggered ChunkRadiusUpdated/BiomeDefinitionList/PlayStatus/
    // CreativeContent/.../chunks/inventory burst, etc.) rather than once
    // per packet.
    if (state.pendingBatch.empty()) {
        return;
    }

    vector<uint8_t> batch = std::move(state.pendingBatch);
    state.pendingBatch.clear();

    // A THIRD attempt at this byte, this time backed by direct evidence
    // instead of theory: the packet-logger capture showed the real
    // client's own outgoing compressed batch starts with 0x00 (which
    // fails to decompress as-is) followed immediately by a genuinely
    // valid raw-deflate stream. That 0x00 matches algorithm=0/zlib
    // exactly. The two prior reverted attempts were both tested while
    // NetworkSettings still negotiated CompressionAlgorithm=None -- this
    // one is scoped to only fire once state.compressionEnabled is true
    // (real compression actually negotiated), which neither prior attempt
    // was testing for, so the earlier regressions don't apply here.

    // Compress-then-encrypt, mirroring PacketHandler.cpp's decrypt-then-
    // decompress on the way in. NetworkSettingsPacket.cpp now negotiates
    // real Zlib compression (previously always "none"), so this is the
    // first place that actually needs to act on state.compressionEnabled
    // for outgoing data -- it was true in name only before.
    if (state.compressionEnabled) {
        vector<uint8_t> compressed;
        vector<uint8_t> withAlgoId;
        if (zlibCompress(batch, compressed)) {
            withAlgoId.reserve(compressed.size() + 1);
            withAlgoId.push_back(0x00); // algorithm id = zlib, matching NetworkSettingsPacket.cpp's negotiated value
            withAlgoId.insert(withAlgoId.end(), compressed.begin(), compressed.end());
        } else {
            // zlibCompress() itself failing (as opposed to just not being
            // worth it size-wise, which zlib doesn't treat as failure) is
            // rare, but state.compressionEnabled being true means the
            // client unconditionally expects a leading algorithm-id byte
            // on every batch from here on -- sending the batch with no
            // prefix at all when this fallback fires would desync the
            // client's parse from byte one. 255 (0xFF) is the same
            // "not compressed" id the real client itself uses for its own
            // small, deliberately-uncompressed batches (confirmed via
            // capture) -- reuse it here so the receiving side's existing
            // algoId==255 raw-passthrough branch (PacketHandler.cpp)
            // handles this correctly too.
            cout << "[GamePacketSender] zlib compression failed, sending batch uncompressed (algoId=255)\n";
            withAlgoId.reserve(batch.size() + 1);
            withAlgoId.push_back(0xFF);
            withAlgoId.insert(withAlgoId.end(), batch.begin(), batch.end());
        }
        batch = std::move(withAlgoId);
    }

    // Once the login encryption handshake has run for this client (see
    // HandshakePacket.cpp), the WHOLE combined batch gets AES-256-CFB8
    // encrypted as one unit, with one 8-byte checksum trailer for the
    // batch -- not one trailer per packet. Encrypting multiple packets
    // together instead of separately is exactly the point of this change.
    if (state.encryptionEnabled) {
        encryptGamePacket(state, batch); // appends trailer, encrypts in place
    }

    vector<uint8_t> payload;
    payload.push_back(0xFE);
    payload.insert(payload.end(), batch.begin(), batch.end());

    // Per-frame RakNet overhead for a Reliable-Ordered (and possibly split)
    // frame: 4 bytes datagram header + 1 byte frame flags + 2 bytes bit-length
    // + 3 bytes reliable index + 4 bytes ordering index/channel + 10 bytes
    // split-packet info (splitCount + splitId + splitIndex), plus the 28
    // bytes of UDP/IP headers the negotiated MTU is meant to leave room for.
    // Rounded up a bit further as a safety margin.
    constexpr size_t kFrameOverhead = 4 + 1 + 2 + 3 + 4 + 10 + 28 + 16;
    const size_t maxFragmentSize = (state.mtu > kFrameOverhead)
        ? (size_t)state.mtu - kFrameOverhead
        : 1024;

    // Small batches fit in one datagram. Large ones (StartGame+ItemRegistry
    // combined, the RequestChunkRadius burst, ...) get split across
    // multiple RakNet split-packet frames, exactly like a real Bedrock
    // server does.
    if (payload.size() <= maxFragmentSize) {
        Frame frame;
        frame.Reliability     = 3; // ReliableOrdered
        frame.isSplit         = false;
        frame.reliableIndex   = state.nextReliableIndex++;
        frame.orderingIndex   = state.nextOrderingIndex++;
        frame.orderingChannel = 0;
        frame.buffer          = payload;

        FrameSetPacket wrapper;
        wrapper.flags          = 0x84;
        wrapper.sequenceNumber = state.nextSeqNum++;
        wrapper.frames.push_back(frame);

        vector<uint8_t> encoded = encodeFrameSetPacket(wrapper);
        state.sentPackets[wrapper.sequenceNumber] = encoded;

        sendto(sock, encoded.data(), encoded.size(), 0,
               (sockaddr*)&clientAddr, sizeof(clientAddr));
        return;
    }

    const uint32_t splitCount = (uint32_t)((payload.size() + maxFragmentSize - 1) / maxFragmentSize);
    const uint16_t splitId    = state.nextSplitId++;
    // All fragments of one oversized batch belong to the same logical
    // ordered message, so they share one ordering index — only the
    // reassembled batch occupies a slot in the ordering channel, the same
    // way the receiving side in PacketHandler.cpp reassembles by splitIndex
    // before ever looking at ordering.
    const uint32_t orderingIndex = state.nextOrderingIndex++;

    cout << "[GamePacketSender] Payload " << payload.size() << " bytes exceeds "
         << maxFragmentSize << "-byte budget for MTU=" << state.mtu
         << "; splitting into " << splitCount << " fragments (splitId="
         << splitId << ")\n";

    for (uint32_t i = 0; i < splitCount; i++) {
        size_t start = (size_t)i * maxFragmentSize;
        size_t end   = min(start + maxFragmentSize, payload.size());

        Frame frame;
        frame.Reliability     = 3; // ReliableOrdered
        frame.isSplit         = true;
        frame.reliableIndex   = state.nextReliableIndex++; // unique per fragment (each is its own reliable datagram)
        frame.orderingIndex   = orderingIndex;              // shared across all fragments of this message
        frame.orderingChannel = 0;
        frame.splitCount      = splitCount;
        frame.splitId         = splitId;
        frame.splitIndex      = i;
        frame.buffer          = vector<uint8_t>(payload.begin() + start, payload.begin() + end);

        FrameSetPacket wrapper;
        wrapper.flags          = 0x84;
        wrapper.sequenceNumber = state.nextSeqNum++;
        wrapper.frames.push_back(frame);

        vector<uint8_t> encoded = encodeFrameSetPacket(wrapper);
        state.sentPackets[wrapper.sequenceNumber] = encoded;

        sendto(sock, encoded.data(), encoded.size(), 0,
               (sockaddr*)&clientAddr, sizeof(clientAddr));

        // Real RakNet implementations never fire a burst of datagrams as
        // fast as the syscalls allow -- they queue outgoing data and flush
        // it on a periodic tick instead (go-raknet's SendInterval, and
        // RakNet's own congestion-control notes, both use ~50ms). A small
        // per-fragment delay keeps this server's send rate sane instead of
        // dumping the whole payload on the network in one instant.
        if (i + 1 < splitCount) {
            this_thread::sleep_for(chrono::milliseconds(2));
        }
    }
}

void sendGamePacket(int sock, sockaddr_in clientAddr, ClientState& state,
                     const vector<uint8_t>& packetBody) {
    // Kept for any call site that genuinely needs to send a single packet
    // immediately on its own (queue + flush back to back). Prefer
    // queueGamePacket() + an explicit flushGamePackets() at the end of a
    // related group of packets wherever possible -- see the comment in
    // flushGamePackets() for why that grouping is what actually matches
    // real server behavior.
    queueGamePacket(state, packetBody);
    flushGamePackets(sock, clientAddr, state);
}
