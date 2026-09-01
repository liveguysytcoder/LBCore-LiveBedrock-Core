#include "SetSpawnPositionPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendSetSpawnPosition(int sock, sockaddr_in clientAddr, ClientState& state,
                           SpawnPositionType type,
                           int32_t x, int32_t y, int32_t z,
                           int32_t dimension) {
    // Gamepacket header (varint) for SetSpawnPosition = 43 (0x2B).
    vector<uint8_t> packet;
    writeVarInt(packet, 43);

    writeZigZag32(packet, static_cast<int32_t>(type)); // 0: Spawn Position Type

    // 1: Block Position (BlockPos)
    writeZigZag32(packet, x);
    writeZigZag32(packet, y);
    writeZigZag32(packet, z);

    writeZigZag32(packet, dimension); // 2: Dimension type (DimensionType.value)

    // 3: Spawn Block Pos (BlockPos) — same as Block Position above; the
    // doc lists it as a second, separate field rather than implying it's
    // always identical, but this project has no distinct "spawn chunk"
    // concept yet, so it mirrors field 1.
    writeZigZag32(packet, x);
    writeZigZag32(packet, y);
    writeZigZag32(packet, z);

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent SetSpawnPosition\n";
}
