#include <iostream>
#include <iomanip>
#include <vector>
#include <arpa/inet.h>
#include <utility>

#include "../Packets/UnconnectedPing.h"
#include "../Packets/OpenConnectionRequest1.h"
#include "../Packets/OpenConnectionRequest2.h"
#include "../Packets/FrameSetPacket.h"
#include "../Packets/AcknowledgePacket.h"
#include "../Packets/ConnectionRequest.h"
#include "../Packets/ConnectedPong.h"
#include "Clients.h"
#include "PacketHandler.h"
#include "../../LBBNet/Server/PacketHandler.h"

using namespace std;

// Handles a single, complete (non-split, or already-reassembled) inner frame
// buffer for one specific client connection.
void processFrameContent(int sock, sockaddr_in clientAddr, ClientState& state, const vector<uint8_t>& buffer) {
    if (buffer.empty()) return;

    cout << "[Frame] ID: 0x" << hex << (int)buffer[0] << dec << "\n";
    switch (buffer[0]) {
        case 0x09: {
            decodeConnectionRequest(sock, buffer.data(), buffer.size(), clientAddr, state);
            break;
        }
        case 0x13:
            cout << "NewIncomingConnection packet had arrived!!!\n";
            break;
        case 0x15:
            cout << "[RakNet] ID_DISCONNECTION_NOTIFICATION received — client is closing the "
                    "connection cleanly (this is a client-initiated RakNet-level disconnect, "
                    "not a server crash or malformed batch)\n";
            removeClient(clientAddr);
            break;
        case 0x00: {
            uint64_t pingTime = 0;
            for (int i = 1; i <= 8; i++)
                pingTime = (pingTime << 8) | buffer[i];
            sendConnectedPong(sock, clientAddr, state, pingTime);
            break;
        }
        case 0x03:
            cout << "[RakNet] Connected Pong received\n";
            break;
        case 0xFE: {
            cout << "[Minecraft] Protocol Packet Received!\n";
            handleBedrockPacket(sock, clientAddr, state, buffer);
            break;
        }
        default:
            cout << "Unknown Packet from inner frame\n";
            break;
    }
}

// Enforces actual Reliable-Ordered delivery for ordering channel 0 (the
// only channel this project's sender uses -- GamePacketSender.cpp). Only
// meant to be called for frames with Reliability == 3 (ReliableOrdered);
// unreliable frames like ConnectedPing bypass this entirely and go
// straight to processFrameContent(), matching how a real client actually
// sends them (confirmed: ConnectedPing arrives with Reliability 0, not 3).
//
// A payload whose orderingIndex is exactly the next one this client is
// expecting gets delivered immediately, then the buffer is drained for any
// later payloads that arrived early and can now proceed in sequence too.
// A payload that arrives early (index > expected) is buffered, not
// delivered, until the gap in front of it closes. A payload at or below an
// index already delivered is a stale duplicate/retransmission and is
// dropped -- this doubles as the duplicate-frame detection this project
// was also missing.
void deliverOrdered(int sock, sockaddr_in clientAddr, ClientState& state,
                     uint32_t orderingIndex, const vector<uint8_t>& payload) {
    if (orderingIndex < state.nextExpectedOrderingIndex) {
        cout << "[Ordering] Dropping stale/duplicate frame, orderingIndex="
             << orderingIndex << " (already past " << state.nextExpectedOrderingIndex << ")\n";
        return;
    }

    if (orderingIndex > state.nextExpectedOrderingIndex) {
        cout << "[Ordering] Buffering out-of-order frame, orderingIndex="
             << orderingIndex << " (expecting " << state.nextExpectedOrderingIndex << ")\n";
        state.orderedFrameBuffer[orderingIndex] = payload;
        return;
    }

    // orderingIndex == nextExpectedOrderingIndex: deliver it now, then
    // drain anything buffered that's now next in line.
    processFrameContent(sock, clientAddr, state, payload);
    state.nextExpectedOrderingIndex++;

    while (true) {
        auto it = state.orderedFrameBuffer.find(state.nextExpectedOrderingIndex);
        if (it == state.orderedFrameBuffer.end()) {
            break;
        }
        cout << "[Ordering] Releasing buffered frame, orderingIndex="
             << state.nextExpectedOrderingIndex << "\n";
        vector<uint8_t> buffered = std::move(it->second);
        state.orderedFrameBuffer.erase(it);
        processFrameContent(sock, clientAddr, state, buffered);
        state.nextExpectedOrderingIndex++;
    }
}

// Main dispatcher
void decodePacket(int sock, sockaddr_in clientAddr, const vector<unsigned char>& data)
{
    if (data.empty()) return;

    unsigned char packetID = data[0];

    cout << "\n[PacketHandler] Packet ID: 0x"
         << hex << setw(2) << setfill('0') << (int)packetID
         << dec << endl;

    switch (packetID)
    {
        case 0x01:
            decodeUnconnectedPing(sock, clientAddr, data);
            break;

        case 0x05:
            decodeOpenConnectionRequestOne(sock, clientAddr, data);
            break;

        case 0x07:
            decodeOpenConnectionRequestTwo(sock, clientAddr, data);
            break;

        case 0xC0: {
            // Client ACKing one of our datagrams. Non-creating lookup: an
            // ACK for a client that no longer has a session (e.g. arrived
            // after disconnect) is stale and gets dropped, not resurrected.
            ClientState* state = findExistingClient(clientAddr);
            if (!state) {
                cout << "[ACK] Ignored — no active session for this address\n";
                break;
            }
            uint32_t ackedSeq = data[4] | (data[5] << 8) | (data[6] << 16);
            state->sentPackets.erase(ackedSeq);
            cout << "[ACK] Received\n";
            break;
        }

        case 0xA0: {
            // Client NACKing one of our datagrams: resend it if we still have it.
            ClientState* state = findExistingClient(clientAddr);
            if (!state) {
                cout << "[NACK] Ignored — no active session for this address\n";
                break;
            }
            uint32_t missingSeq = data[4] | (data[5] << 8) | (data[6] << 16);
            auto it = state->sentPackets.find(missingSeq);
            if (it != state->sentPackets.end()) {
                sendto(sock, it->second.data(), it->second.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                cout << "[NACK] Resent SeqNum =" << missingSeq << "\n";
            } else {
                cout << "[NACK] SeqNum =" << missingSeq << " not in buffer!\n";
            }
            break;
        }

        default:

            // RakNet FrameSet packets (0x80 - 0x8D)
            if ((packetID & 0xF0) == 0x80) {
                // Non-creating lookup. The session is established during the
                // OpenConnectionRequest2 handshake (before any FrameSet can
                // legitimately arrive), so a FrameSet for an unknown address
                // is a stray/retransmitted frame from an already-closed
                // connection (e.g. a duplicate disconnect notice arriving
                // after we already processed and erased that client's
                // state) — drop it instead of spinning up a phantom new
                // client and re-running its contents (which previously
                // caused repeated "New client ... assigned index N" churn
                // and re-processing of a disconnect that already happened).
                ClientState* statePtr = findExistingClient(clientAddr);
                if (!statePtr) {
                    cout << "[FrameSet] Ignored — no active session for this address "
                            "(stray/duplicate frame after disconnect)\n";
                    break;
                }
                ClientState& state = *statePtr;

                FrameSetPacket packet = decodeFrameSetPacket(data.data(), data.size());
                printFrameSetPacket(packet);

                sendACKPacket(sock, clientAddr, state, packet.sequenceNumber);
                checkAndSendNACK(sock, clientAddr, state, packet.sequenceNumber);

                for (auto& frame : packet.frames) {

                    if (frame.isSplit) {
                        SplitPacketBuffer& buf = state.splitBuffers[frame.splitId];
                        buf.splitCount = frame.splitCount;
                        buf.fragments[frame.splitIndex] = frame.buffer;

                        cout << "[Split] Got fragment " << frame.splitIndex
                             << "/" << frame.splitCount
                             << " for splitId=" << frame.splitId << "\n";

                        if (buf.fragments.size() < buf.splitCount) {
                            // Not all fragments have arrived yet, wait for more
                            continue;
                        }

                        // All fragments present: reassemble in order
                        vector<uint8_t> reassembled;
                        for (uint32_t i = 0; i < buf.splitCount; i++) {
                            const vector<uint8_t>& part = buf.fragments[i];
                            reassembled.insert(reassembled.end(), part.begin(), part.end());
                        }

                        cout << "[Split] Reassembled packet, total size = "
                             << reassembled.size() << " bytes\n";

                        state.splitBuffers.erase(frame.splitId);

                        if (frame.Reliability == 3) {
                            deliverOrdered(sock, clientAddr, state, frame.orderingIndex, reassembled);
                        } else {
                            processFrameContent(sock, clientAddr, state, reassembled);
                        }
                        continue;
                    }

                    if (frame.Reliability == 3) {
                        deliverOrdered(sock, clientAddr, state, frame.orderingIndex, frame.buffer);
                    } else {
                        processFrameContent(sock, clientAddr, state, frame.buffer);
                    }
                }
            }
            else
            {
                cout << "[PacketHandler] Unknown packet. Raw bytes:\n";

                for (unsigned char b : data)
                    printf("%02X ", b);

                cout << endl;
            }

            break;
    }
}
