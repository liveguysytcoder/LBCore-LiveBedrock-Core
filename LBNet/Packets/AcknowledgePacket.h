#pragma once
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <set>
#include <cstdint>
#include "../Server/Clients.h"

using namespace std;

const uint32_t MAX_SEQ = 0xFFFFFF;

// Sends an ACK for a single received sequence number. `state` is this
// client's own connection state, so acked-sequence bookkeeping never leaks
// between clients.
void sendACKPacket(int sock, sockaddr_in clientAddr, ClientState& state, uint32_t seqNum);

// Looks at the gap between the last sequence number we expected and the one
// we just received, and sends a NACK for every sequence number in between
// that we haven't already seen acked.
void checkAndSendNACK(int sock, sockaddr_in clientAddr, ClientState& state, uint32_t receivedSeq);

void sendNACK(int sock, sockaddr_in clientAddr, uint32_t seqNum);
