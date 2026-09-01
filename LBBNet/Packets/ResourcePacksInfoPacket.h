#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends ResourcePacksInfo (0x06) to the client with empty pack lists. We
// don't serve any resource/behavior packs yet, but sending this at all is
// what lets the client move on (ResourcePackClientResponse) instead of
// sitting there waiting for a reply that never comes.
void sendResourcePacksInfo(int sock, sockaddr_in clientAddr, ClientState& state);
