#include "HandshakePacket.h"
#include "GamePacketSender.h"
#include "DataTypeHelper.h"
#include "Encryption.h"
#include "ResourcePacksInfoPacket.h"
#include "PlayStatusPacket.h"
#include <openssl/rand.h>
#include <iostream>

using namespace std;

void sendServerToClientHandshake(int sock, sockaddr_in clientAddr, ClientState& state,
                                  const vector<uint8_t>& clientPublicKeyDer) {
    vector<uint8_t> secret = deriveSharedSecret(clientPublicKeyDer);
    if (secret.empty()) {
        // Couldn't parse the client's public key / derive a secret — rather
        // than hang the connection, fall back to the old unencrypted flow.
        // A real official client won't accept this, but it keeps things
        // working for anything that sent a malformed key.
        cout << "[Handshake] Could not derive a shared secret from the client's public "
                "key — continuing without encryption\n";
        sendResourcePacksInfo(sock, clientAddr, state);
        return;
    }

    vector<uint8_t> salt(16);
    RAND_bytes(salt.data(), static_cast<int>(salt.size()));

    vector<uint8_t> encryptionKey = computeEncryptionKey(salt, secret);
    string jwt = buildHandshakeJwt(salt);

    // Gamepacket body: varint header (packet ID 0x03) + a single String
    // field carrying the JWT.
    vector<uint8_t> body;
    writeVarInt(body, 0x03);
    writeString(body, jwt);

    // This is the last packet sent unencrypted — sendGamePacket checks
    // state.encryptionEnabled, which is still false here.
    sendGamePacket(sock, clientAddr, state, body);

    // From this point on, both directions are encrypted: the client turns
    // encryption on as soon as it validates this JWT, and its next packet
    // (ClientToServerHandshake) will already be encrypted when it arrives.
    initClientEncryption(state, encryptionKey);

    cout << "[Handshake] Sent ServerToClientHandshake — encryption is now enabled "
            "for both directions on this connection\n";
}

void handleClientToServerHandshake(int sock, sockaddr_in clientAddr, ClientState& state) {
    cout << "[Handshake] ClientToServerHandshake received — encryption confirmed, "
            "continuing login\n";

    // Per the official Bedrock login sequence (minecraft.wiki's "Bedrock
    // Login Sequence" page):
    //   C->S : ClientToServerHandshakePacket
    //   S->C : PlayStatusPacket(status=LOGIN_SUCCESS)
    //   (Resource Pack Handling)
    //   S->C : ResourcePacksInfoPacket
    //   ...
    // PlayStatus(LoginSuccess) is the packet that closes out the Login
    // stage and belongs right here, immediately after
    // ClientToServerHandshake and before ResourcePacksInfo — not after
    // ResourcePacksStack, which is one stage later (Resource Pack
    // Handling, not Login).
    sendPlayStatus(sock, clientAddr, state, PlayStatusType::LoginSuccess);
    sendResourcePacksInfo(sock, clientAddr, state);
}
