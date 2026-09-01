#include "NetworkChunkPublisherUpdatePacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendNetworkChunkPublisherUpdate(int sock, sockaddr_in clientAddr, ClientState& state,
                                      int32_t x, int32_t y, int32_t z, uint32_t radiusBlocks) {
    vector<uint8_t> packet;
    writeVarInt(packet, 121); // NetworkChunkPublisherUpdate

    writeZigZag32(packet, x);
    writeZigZag32(packet, y);
    writeZigZag32(packet, z);
    writeVarInt(packet, radiusBlocks);

    // FIXED: confirmed against Mojang's official protocol docs (protocol
    // 2168 / 1.26.44) -- the "Server Built Chunks List" array is
    // explicitly annotated "No size compression", meaning its count is a
    // plain 4-byte little-endian uint32, NOT the usual varuint32 used by
    // every other collection in the protocol. Writing a 1-byte varint(0)
    // here instead of a 4-byte uint32(0) is exactly what the js-client
    // capture's "lu32" (little-endian uint32) reader choked on -- it tried
    // to read 4 raw bytes for the count and ran off the end of the packet.
    writeUInt(packet, 0); // SavedChunks array count — none; we send LevelChunk directly instead

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent NetworkChunkPublisherUpdate(" << x << ", " << y << ", " << z
         << ", radius=" << radiusBlocks << ")\n";
}
