#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends UpdatePlayerGameType (151 / 0x97) — confirms a specific player's
// (by unique id) gamemode, distinct from SetPlayerGameType which only
// covers "the local player". Real capture fields, in order:
//   gamemode           ZigZag VarInt32
//   player_unique_id   ZigZag VarInt64
//   tick                    VarInt64 (unsigned)
void sendUpdatePlayerGameType(int sock, sockaddr_in clientAddr, ClientState& state,
                               int32_t gamemode, int64_t playerUniqueId, uint64_t tick);
