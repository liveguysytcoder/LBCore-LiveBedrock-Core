#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Identity fields pulled out of the Login packet's certificate chain.
struct LoginIdentity {
    bool valid = false;
    string identityUUID; // "identity" claim — the player's Mojang/Xbox UUID
    string displayName;
    string xuid;

    // "identityPublicKey" claim — the client's P-384 public key, base64
    // DER. Present whenever the client logged in with a real Xbox Live
    // identity (an official client, signed in and online); empty for the
    // older offline/no-chain logins this project also still accepts. Its
    // presence is what decides whether handleLogin() runs the encryption
    // handshake (see HandshakePacket.h) before continuing.
    string publicKeyBase64;
};

// Scans every JWT in the chain's JSON (`{"chain":["<jwt>", ...]}`) for the
// one carrying an "extraData" claim, and pulls XUID/displayName/identity
// out of it. Returns a LoginIdentity with valid=false if none is found.
LoginIdentity extractLoginIdentity(const string& chainJson);

// Parses the Login (0x01) game packet body — offset must already point
// past the varint gamepacket header, i.e. at the start of the protocol
// version field — extracts the player's identity from the certificate
// chain, stores it on the client's state, and sends PlayStatus +
// ResourcePacksInfo back so the handshake continues instead of stalling.
void handleLogin(int sock, sockaddr_in clientAddr, ClientState& state,
                  const vector<uint8_t>& data, size_t offset);
