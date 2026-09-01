#pragma once
#include <vector>
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Wraps an already-encoded game packet (gamepacket-header varint + body) in
// the 0xFE batch marker + varint length, puts that in a reliable-ordered
// FrameSetPacket, and sends it to the client. This is the same wrapping
// sendNetworkSettings() in NetworkSettingsPacket.cpp does inline; pulling it
// out here means every packet sent after NetworkSettings (PlayStatus,
// ResourcePacksInfo, ...) only has to build its own packet body.
void sendGamePacket(int sock, sockaddr_in clientAddr, ClientState& state,
                     const vector<uint8_t>& packetBody);

// Queues a packet body onto state.pendingBatch without sending anything.
// Confirmed against gophertunnel's real Conn.WritePacket(), which only ever
// buffers -- call flushGamePackets() to actually combine everything queued
// since the last flush into one encrypted batch and send it. Prefer this
// pairing over sendGamePacket() for any group of packets that logically
// belong together (StartGame+ItemRegistry, a ResourcePackClientResponse
// reply, the RequestChunkRadius-triggered spawn burst, ...) -- see the
// comment in flushGamePackets() (GamePacketSender.cpp) for why that
// grouping is what real servers actually do on the wire.
void queueGamePacket(ClientState& state, const vector<uint8_t>& packetBody);

// Sends everything queued via queueGamePacket() since the last flush, as
// ONE combined encrypted batch. No-op if nothing is queued.
void flushGamePackets(int sock, sockaddr_in clientAddr, ClientState& state);
