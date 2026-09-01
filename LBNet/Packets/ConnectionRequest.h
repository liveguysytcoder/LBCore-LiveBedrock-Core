#pragma once
#include <iostream>
#include <cstdint>
#include "../Server/Clients.h"

using namespace std;

struct ConnectionRequest {
    uint8_t id;
    uint64_t clientGuid;
    uint64_t incomingTimeStamp;
    bool doSecurity;
    uint8_t clientProof[32];
    bool doIdentity;
    uint8_t identityProof[294];
};

ConnectionRequest decodeConnectionRequest(int sock, const uint8_t* data, size_t length, sockaddr_in clientAddr, ClientState& state);
