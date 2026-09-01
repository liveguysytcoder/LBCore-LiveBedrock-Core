#pragma once
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

void sendNetworkSettings(int sock, sockaddr_in clientAddr, ClientState& state);
void printNetworkSettingsLayers(const vector<uint8_t>& packet, const vector<uint8_t>& payload);
