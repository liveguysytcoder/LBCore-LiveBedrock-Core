#pragma once
#include <vector>
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends ServerToClientHandshake (0x03): a JWT containing the server's P-384
// public key and a random salt, signed with the server's private key. This
// is the first step of the login encryption handshake — only reached when
// the client logged in with a real Xbox Live identity (see LoginPacket.cpp,
// which checks for the login JWT's "identityPublicKey" claim). An official
// Minecraft client signed in and playing online always expects this before
// it will continue past Login.
//
// `clientPublicKeyDer` is the client's own public key (already base64-
// decoded by the caller) — used here to derive the shared secret via ECDH.
//
// Sent unencrypted, same as everything before it. Immediately after
// sending it, this function turns encryption on for both directions on
// `state` — every packet after this one (including the client's reply) is
// expected to be encrypted.
void sendServerToClientHandshake(int sock, sockaddr_in clientAddr, ClientState& state,
                                  const vector<uint8_t>& clientPublicKeyDer);

// Handles ClientToServerHandshake (0x04) — the client's empty
// acknowledgement that it has encryption enabled on its end too. By the
// time this is called the packet has already been decrypted (see
// LBBNet/Server/PacketHandler.cpp's handleBedrockPacket), so there's
// nothing left to parse; this just continues the login sequence exactly as
// it did for offline logins (PlayStatus + ResourcePacksInfo), just
// encrypted now.
void handleClientToServerHandshake(int sock, sockaddr_in clientAddr, ClientState& state);
