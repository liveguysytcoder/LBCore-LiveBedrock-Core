#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends CreativeContent (145 / 0x91) to the client — the contents of the
// Creative inventory/recipe book. This project defines no creative groups
// or item entries yet, so both lists are sent empty (an empty Creative
// tab, rather than omitting the packet — a real client still expects it).
void sendCreativeContent(int sock, sockaddr_in clientAddr, ClientState& state);
