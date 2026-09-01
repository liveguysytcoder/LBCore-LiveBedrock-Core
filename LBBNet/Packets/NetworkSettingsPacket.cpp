#include "NetworkSettingsPacket.h"
#include "../../LBNet/Packets/FrameSetPacket.h"
#include "DataTypeHelper.h"
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

void sendNetworkSettings(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Layer 1: NetworkSettings packet content.
    // Gamepacket header (10-bit id + 2-bit sender subclient + 2-bit target
    // subclient, packed into a varint). We don't support split-screen
    // subclients, so both subclient fields are 0 and the header is just the
    // packet id itself.
    vector<uint8_t> packet;
    writeVarInt(packet, 0x8F);
    // Switched from CompressionAlgorithm=none to real Zlib compression.
    // "None" is valid per the wiki, but a bedrock-protocol (npm) test
    // client crashed trying to build its own Login reply against it --
    // tried to apply algorithm 0xFFFF as if it were a real codec, got
    // nothing back, and hit a Buffer.concat error on an undefined
    // compressed buffer. Raising the threshold alone didn't help, since
    // the client's crash was about the algorithm id itself, not the
    // threshold. Zlib is what real production Bedrock servers use (see
    // the wiki's own compression section and Compression.h/.cpp's
    // existing zlib helpers, previously unused), and every client
    // implementation is expected to support it, so this sidesteps the
    // "none" edge case entirely instead of relying on every client
    // handling it correctly.
    writeUShort(packet, 1);       // CompressionThreshold — real BDS default, compress anything above 1 byte
    writeUShort(packet, 0x0000);  // CompressionAlgorithm = Zlib
    writeBool(packet, false);
    writeUByte(packet, 0);
    writeFloat(packet, 0.0f);

    // Layer 2: game packet batch wrapper.
    //
    // Correction: an earlier pass at this file inserted an extra
    // "compression id" byte here based on a misreading of the wiki's WIP
    // Compression section. Captured traffic from a real client proves that's
    // wrong — its RequestNetworkSettings frame is exactly
    // `0xFE | varint(length) | packetBytes`, with no id byte at all. The
    // negotiated compression algorithm (from NetworkSettings) applies to the
    // whole connection from then on; it isn't re-signalled per batch on the
    // wire. Since we always negotiate "none", the packet bytes go out as-is;
    // see Compression.h/.cpp for when a real algorithm needs to be applied.
    vector<uint8_t> payload;
    payload.push_back(0xFE);
    writeVarInt(payload, (uint32_t)packet.size());
    payload.insert(payload.end(), packet.begin(), packet.end());

    printNetworkSettingsLayers(packet, payload);

    Frame frame;
    frame.Reliability     = 3;
    frame.isSplit         = false;
    frame.reliableIndex   = state.nextReliableIndex++;
    frame.orderingIndex   = state.nextOrderingIndex++;
    frame.orderingChannel = 0;
    frame.buffer          = payload;

    FrameSetPacket wrapper;
    wrapper.flags          = 0x84;
    wrapper.sequenceNumber = state.nextSeqNum++;
    wrapper.frames.push_back(frame);

    printEncodedFrameSetPacket(wrapper);

    vector<uint8_t> encoded = encodeFrameSetPacket(wrapper);
    state.sentPackets[wrapper.sequenceNumber] = encoded;

    sendto(sock, encoded.data(), encoded.size(), 0,
           (sockaddr*)&clientAddr, sizeof(clientAddr));

    cout << "Sent NetworkSettings(" << encoded.size() << " bytes)\n";
    cout << "Buffer (hex):\n  ";
    for (size_t i = 0; i < encoded.size(); i++)
        printf("%02x ", encoded[i]);
    printf("\n");

    // Bedrock's compression negotiation is server-decided and takes effect
    // immediately for every batch after this one -- there's no ack from the
    // client to wait for. Both directions (PacketHandler.cpp's incoming
    // decompress, GamePacketSender.cpp's outgoing compress) key off this
    // flag, so it must flip the moment NetworkSettings actually goes out.
    state.compressionEnabled = true;
}

void printNetworkSettingsLayers(const vector<uint8_t>& packet, const vector<uint8_t>& payload) {
    // Layer 1
    cout << "===========================================\n";
    cout << "NetworkSettings Packet (Layer 1)\n";
    cout << "===========================================\n";
    // The game packet header (0x8F) is written with writeVarInt, and 0x8F
    // (143) is >= 0x80, so it actually takes 2 bytes on the wire (0x8F 0x01),
    // not 1. Reading the fields below at a fixed +1 offset silently pulled
    // in the wrong bytes (e.g. logging CompressionThreshold as 257 and
    // CompressionAlgorithm as 65280 instead of the real 1 / 65535) even
    // though the bytes actually sent to the client were correct. Skip past
    // the varint properly instead of assuming its width.
    size_t headerOffset = 1;
    while (headerOffset < packet.size() && (packet[headerOffset - 1] & 0x80))
        headerOffset++;

    cout << "  Packet ID:               0x8F\n";
    cout << "  CompressionThreshold:    " << (*(uint16_t*)(packet.data() + headerOffset)) << "\n";
    cout << "  CompressionAlgorithm:    " << (*(uint16_t*)(packet.data() + headerOffset + 2)) << "\n";
    cout << "  ClientThrottleEnabled:   " << (packet[headerOffset + 4] ? "true" : "false") << "\n";
    cout << "  ClientThrottleThreshold: " << (int)packet[headerOffset + 5] << "\n";
    cout << "  ClientThrottleScalar:    " << (*(float*)(packet.data() + headerOffset + 6)) << "\n";
    cout << "  Size: " << packet.size() << " bytes\n";
    cout << "===========================================\n";

    // Layer 2 — read real values from payload bytes.
    // payload[0]      = 0xFE marker
    // payload[1..n]   = VarInt length of the packet that follows
    // payload[n+1..]  = packet bytes
    size_t offset = 1; // skip 0xFE
    uint32_t varlen = 0;
    int shift = 0;
    while (offset < payload.size()) {
        uint8_t b = payload[offset++];
        varlen |= (uint32_t)(b & 0x7F) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }

    cout << "===========================================\n";
    cout << "Batch Wrapper (Layer 2)\n";
    cout << "===========================================\n";
    cout << "  Batch Marker:           0x" << hex << (int)payload[0] << dec << "\n";
    cout << "  Packet Length (VarInt): " << varlen << "\n";
    cout << "  Total Size:             " << payload.size() << " bytes\n";
    cout << "===========================================\n";
}
