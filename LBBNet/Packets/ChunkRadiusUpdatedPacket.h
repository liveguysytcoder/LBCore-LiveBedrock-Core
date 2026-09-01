#pragma once
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends ChunkRadiusUpdated (70 / 0x46) to the client -- the server's reply
// to the client's RequestChunkRadius, confirming the actual view distance
// (in chunks) it should use. Real servers are expected to answer this;
// this project never read RequestChunkRadius at all before, so this was
// never sent.
void sendChunkRadiusUpdated(int sock, sockaddr_in clientAddr, ClientState& state, int32_t chunkRadius);
