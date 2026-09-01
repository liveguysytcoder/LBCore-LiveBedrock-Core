#pragma once
#include <cstdint>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// One requested sub-chunk, as parsed from the client's SubChunkRequest.
struct SubChunkOffsetRequest {
    int8_t dx, dy, dz;
};

// Sends SubChunk (161 / 0xA1) in response to a SubChunkRequest: one entry
// per requested offset, echoing dimension/position/offset back and filling
// in real superflat terrain for the one sub-chunk that actually has
// content (absolute Y index 0, i.e. world Y 0-15) and an explicit
// all-air sub-chunk for every other requested offset.
//
// See LevelChunkPacket.cpp's header comment for why this packet exists at
// all now: LevelChunk no longer carries block/biome data directly -- this
// is the packet that actually delivers it, per the client's own request.
void sendSubChunkResponse(int sock, sockaddr_in clientAddr, ClientState& state,
                           int32_t dimension, int32_t baseX, int32_t baseY, int32_t baseZ,
                           const vector<SubChunkOffsetRequest>& offsets);
