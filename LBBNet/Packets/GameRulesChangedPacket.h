#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends GameRulesChanged (72 / 0x48). Real capture sent exactly one rule
// post-spawn: {"name":"dodaylightcycle","editable":false,"type":"bool",
// "value":true}. Per-rule layout:
//   name       String
//   editable   bool
//   type       VarInt enum (Bool=1, Int=2, Float=3)
//   value      union on type -- bool(1 byte) / ZigZagVarInt32 / f32
// This only sends that one bool rule, matching the capture; not a general
// multi-type gamerule API.
void sendGameRulesChanged(int sock, sockaddr_in clientAddr, ClientState& state,
                           const string& name, bool editable, bool value);
