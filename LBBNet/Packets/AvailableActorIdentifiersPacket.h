#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends AvailableActorIdentifiers (119 / 0x77) to the client. Per the doc,
// its single field is a CompoundTag (NBT) containing a list of ActorInfo
// entries (rid/id/bid/hasspawnegg/summonable). This project has no NBT
// library and no custom entity identifiers to advertise, so this sends
// the smallest valid value for that field: an empty NBT compound — same
// approach StartGamePacket's Player Property Data uses.
void sendAvailableActorIdentifiers(int sock, sockaddr_in clientAddr, ClientState& state);
