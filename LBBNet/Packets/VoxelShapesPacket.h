#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends VoxelShapesPacket (337 / 0x151) to the client.
//
// Per the protocol doc: "Syncs client with server voxel shape data on
// world join. This packet contains a copy of all behavior pack voxel
// shapes data and is used by StartGamePacket... This packet should
// always be sent before StartGamePacket." We don't serve any behavior
// packs (and therefore define no custom voxel shapes), so this sends
// the packet with every list empty — just enough for the client to
// accept it and move on to StartGame, same approach as the empty
// ResourcePacksInfo/Stack pair.
void sendVoxelShapes(int sock, sockaddr_in clientAddr, ClientState& state);
