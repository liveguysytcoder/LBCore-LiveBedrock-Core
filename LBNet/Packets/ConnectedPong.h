#pragma once
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdint>
#include "../Server/Clients.h"

using namespace std;

void sendConnectedPong(int sock, sockaddr_in clientAddr, ClientState& state, uint64_t pingTime);
