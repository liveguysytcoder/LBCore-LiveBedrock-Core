#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends SetPlayerGameType (62 / 0x3E) — tells the client which gamemode
// the local player is in. Single field: gamemode (ZigZag VarInt32).
// Real capture: sent right after AvailableActorIdentifiers, before
// UpdateAbilities/UpdatePlayerGameType (packet #28 in the reference log).
void sendSetPlayerGameType(int sock, sockaddr_in clientAddr, ClientState& state, int32_t gamemode);
