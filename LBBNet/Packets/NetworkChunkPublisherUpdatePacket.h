#pragma once
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends NetworkChunkPublisherUpdate (121 / 0x79): tells the client which
// block position is the center of its currently-loaded chunk radius. This
// is what actually makes the client start rendering/using the chunk(s) it
// was already sent via LevelChunk — without it, LevelChunk data just sits
// unused. radiusBlocks is the publish radius, in blocks.
void sendNetworkChunkPublisherUpdate(int sock, sockaddr_in clientAddr, ClientState& state,
                                      int32_t x, int32_t y, int32_t z, uint32_t radiusBlocks);
