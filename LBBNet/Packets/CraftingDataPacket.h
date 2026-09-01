#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends CraftingData (52 / 0x34) to the client — the server's recipe
// list across all 11 recipe categories (shaped/shapeless/multi/chemistry/
// smithing/potion-mix/container-mix/material-reducer), plus a
// "Clear Recipes" flag. This project defines no custom recipes, so every
// category is sent empty. Clear Recipes is sent false so the client keeps
// its own default vanilla recipes instead of losing the ability to craft
// anything.
void sendCraftingData(int sock, sockaddr_in clientAddr, ClientState& state);
