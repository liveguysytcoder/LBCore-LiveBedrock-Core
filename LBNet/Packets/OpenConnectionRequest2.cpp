#include <iostream>
#include <vector>
#include <arpa/inet.h>

#include "../Server/Clients.h"

using namespace std;

extern const unsigned char RAKNET_MAGIC[16];

// function from reply file
extern void sendOpenConnectionReplyTwo(int sock, sockaddr_in clientAddr, unsigned short mtu);

// From Clients.h — establishes the session state for this connection.
extern uint16_t addClient(sockaddr_in clientAddr);

// Decode Open Connection Request 2 (0x07)
void decodeOpenConnectionRequestTwo(int sock, sockaddr_in clientAddr, const vector<unsigned char>& data)
{
    //cout << "\n🔐 [Open Connection Request 2 Detected]\n";

    if (data.size() < 1 + 16 + 8) {
        cout << "[WARN] Invalid packet size for Open Connection Request 2\n";
        return;
    }

    // verify RakNet magic
    bool magicOK = true;
    for (int i = 0; i < 16; i++) {
        if (data[1 + i] != RAKNET_MAGIC[i])
            magicOK = false;
    }

    //cout << "RakNet Magic Valid: " << (magicOK ? "YES" : "NO") << endl;

    // Extract MTU
    unsigned short mtu = (data[data.size() - 10] << 8) | data[data.size() - 9];
    //cout << "MTU Size: " << mtu << endl;

    // Extract client GUID
    unsigned long long clientGUID = 0;
    for (int i = data.size() - 8; i < data.size(); i++)
        clientGUID = (clientGUID << 8) | data[i];

    //cout << "Client GUID: 0x" << hex << clientGUID << dec << endl;

    if (magicOK) {
        // Establish the session HERE, at the real RakNet handshake step,
        // instead of lazily on the first 0x80 FrameSet. This is what lets
        // decodePacket's FrameSet/ACK/NACK branches use a non-creating
        // lookup: a stray or retransmitted frame arriving for an address
        // with no session (e.g. after that client already disconnected)
        // can be dropped instead of silently spawning a brand-new phantom
        // client entry with its own incrementing index.
        addClient(clientAddr);

        // Remember the negotiated MTU for this client so outgoing game
        // packets know how big a single datagram is allowed to be (see
        // sendGamePacket() in GamePacketSender.cpp, and the mtu field's
        // comment in Clients.h).
        if (ClientState* state = findExistingClient(clientAddr)) {
            state->mtu = mtu;
        }

        sendOpenConnectionReplyTwo(sock, clientAddr, mtu);
    }
}