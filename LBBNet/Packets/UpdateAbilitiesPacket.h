#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends UpdateAbilities (187 / 0xBB) -- grants the player's permission
// level and per-ability flags (fly, build, mine, invulnerable, ...).
// Without this, a real client has no permissions at all and can't
// interact with the world it can see.
//
// CAVEAT: this packet's internal layer/bitset layout is the least-verified
// packet added in this pass -- reconstructed from a real capture's
// DECODED JSON (packet #29), not a byte-level spec. The reference showed
// a single ability layer with every one of its ~20 ability bits set true
// (raw combined value 1048575 = 0xFFFFF), which this sends as one
// "allowed" bitset. If the client rejects/misparses this packet, checking
// node_modules/minecraft-data/data/bedrock/1.21.130/types.yml for
// "AbilityLayer" is the fastest way to get the real field layout.
void sendUpdateAbilities(int sock, sockaddr_in clientAddr, ClientState& state);
