#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends Text (9 / 0x09) — the "X joined the game" translation-key chat
// message a real server sends right after spawn. Real capture (type
// Translation):
//   type                 byte enum (Translation = 2)
//   needs_translation    bool
//   message              String -- a translation KEY, not literal text
//                          ("%multiplayer.player.joined"); the client
//                          looks this up in its own language file
//   parameters           VarInt count + String[] -- fills the key's %s
//   xuid                 String
//   platform_chat_id     String
//   has_filtered_message bool
// MEDIUM confidence: the exact conditional shape of the trailing
// filtered-message field (whether has_filtered_message=false means no
// further bytes, vs. an always-present-but-empty string) isn't confirmed
// byte-for-byte -- this is the LAST field in the packet, so getting it
// wrong can't desync anything else in the batch, worst case is a
// malformed/missing join message, which is why this stayed "low
// priority, cosmetic" rather than blocking.
void sendJoinMessage(int sock, sockaddr_in clientAddr, ClientState& state, const string& playerName);
