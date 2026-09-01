#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends an EMPTY TrimData (302 / 0x12E) — armor-trim smithing-template
// data (patterns[] + materials[]), purely cosmetic (drives the smithing
// table UI). Real capture had 18 patterns + several materials; this
// project has no smithing templates registered at all, so sending zero
// counts for both arrays is consistent with the rest of this project's
// (currently empty) item/crafting registries, same as CreativeContent and
// CraftingData.
void sendEmptyTrimData(int sock, sockaddr_in clientAddr, ClientState& state);
