#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends SetTime (10 / 0x0A) — the world's current time-of-day tick, one
// of the oldest and most stable packets in the protocol. Single field:
// time (ZigZag VarInt32). Real capture sent `time: 1` right after the
// post-spawn UpdateAttributes burst.
void sendSetTime(int sock, sockaddr_in clientAddr, ClientState& state, int32_t time);
