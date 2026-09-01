#pragma once
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

enum class SpawnPositionType : int32_t {
    PlayerRespawn = 0,
    WorldSpawn    = 1,
};

// Sends SetSpawnPosition (43 / 0x2B) to the client, telling it where the
// player (or the world) respawns. Per the doc, field layout is:
//   0: Spawn Position Type (varint32/zigzag, enum SpawnPositionType)
//   1: Block Position      (BlockPos: varint32/zigzag x, y, z)
//   2: Dimension type      (DimensionType: varint32/zigzag value)
//   3: Spawn Block Pos     (BlockPos: varint32/zigzag x, y, z)
void sendSetSpawnPosition(int sock, sockaddr_in clientAddr, ClientState& state,
                           SpawnPositionType type,
                           int32_t x, int32_t y, int32_t z,
                           int32_t dimension);
