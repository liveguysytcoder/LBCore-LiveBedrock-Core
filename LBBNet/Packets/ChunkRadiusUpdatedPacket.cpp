#include "ChunkRadiusUpdatedPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendChunkRadiusUpdated(int sock, sockaddr_in clientAddr, ClientState& state, int32_t chunkRadius) {
    // Gamepacket header (varint) for ChunkRadiusUpdated = 70 (0x46).
    vector<uint8_t> packet;
    writeVarInt(packet, 70);

    writeZigZag32(packet, chunkRadius); // ChunkRadius (signed varint)

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent ChunkRadiusUpdated(radius=" << chunkRadius << ")\n";
}
