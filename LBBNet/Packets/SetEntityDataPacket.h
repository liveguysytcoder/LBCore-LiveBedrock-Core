#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends SetEntityData (39 / 0x27 -- named SetActorData in current Mojang
// docs, decoded as "set_entity_data" by bedrock-protocol/minecraft-data,
// same packet either name) for this project's one player entity.
//
// SIMPLIFIED: a real capture (packet #31) shows this carrying a "flags"
// metadata entry (a packed 64-bit bitset of onfire/sneaking/invisible/
// etc.) plus more entries after it. This project has no entity-flag state
// to report yet, so this sends a validly-framed packet with ZERO metadata
// entries instead of guessing bit positions for a flags value -- still
// tells the client "here is this entity's (empty) metadata", which is
// enough to not be a missing-packet gap, without risking a wrong flags
// bitset making the player render incorrectly (invisible, on fire, etc).
void sendSetEntityData(int sock, sockaddr_in clientAddr, ClientState& state);
