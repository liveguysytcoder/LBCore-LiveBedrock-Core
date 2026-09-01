#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Event IDs this project actually sends — StopRain/StopThunder, matching
// the real capture's post-spawn pair (packets #23/#24, both at (0,0,0)
// with data=0, since weather effects don't use their coordinates). Values
// confirmed against multiple independent PocketMine-derived sources.
enum class LevelEventType : int32_t {
    StopRain = 3003,
    StopThunder = 3004,
};

// Sends LevelEvent (25 / 0x19) — one of the oldest, most stable packets in
// the protocol (world/particle/sound events). Fields: event (ZigZag
// VarInt32), position (3x float32 LE), data (ZigZag VarInt32).
void sendLevelEvent(int sock, sockaddr_in clientAddr, ClientState& state,
                     LevelEventType event, float x, float y, float z, int32_t data);
