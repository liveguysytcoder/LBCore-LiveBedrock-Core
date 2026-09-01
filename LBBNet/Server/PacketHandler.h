#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Decodes one already-decompressed Bedrock game packet (starting at its
// varint header) and dispatches it.
void handleGamePacket(int sock, sockaddr_in clientAddr, ClientState& state, const vector<uint8_t>& data);

// Unwraps a full "0xFE batch" frame buffer (compression id + one or more
// length-prefixed game packets) and dispatches each contained packet.
void handleBedrockPacket(int sock, sockaddr_in clientAddr, ClientState& state, const vector<uint8_t>& data);
