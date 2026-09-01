#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends ResourcePacksStack (0x07) to the client with empty pack lists — the
// server's reply once the client says it has everything it needs (we never
// offered any packs via ResourcePacksInfo, so there's nothing to stack).
// This is what lets the client send its final ResourcePackClientResponse
// (COMPLETED) and move on toward StartGame instead of stalling here.
void sendResourcePacksStack(int sock, sockaddr_in clientAddr, ClientState& state);
