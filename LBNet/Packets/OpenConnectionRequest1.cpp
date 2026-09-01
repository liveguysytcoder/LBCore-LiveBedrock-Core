#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

extern const unsigned char RAKNET_MAGIC[16];

// function from reply file
extern void sendOpenConnectionReplyOne(int sock, sockaddr_in clientAddr, unsigned char protocol);

// Decode Open Connection Request 1 (0x05)
void decodeOpenConnectionRequestOne(int sock, sockaddr_in clientAddr, const vector<unsigned char>& data) {

    //cout << "\n🔓 [Open Connection Request 1 Detected]\n";

    if (data.size() < 1 + 16 + 1) {
        cout << "[WARN] Invalid packet size for Open Connection Request 1.\n";
        return;
    }

    // Verify RakNet magic
    bool magicOK = true;
    for (int i = 0; i < 16; ++i) {
        if (data[1 + i] != RAKNET_MAGIC[i])
            magicOK = false;
    }

    unsigned char protocol = data[17];

    //cout << "Protocol Version: " << (int)protocol << endl;
    //cout << "RakNet Magic Valid: " << (magicOK ? "YES" : "NO") << endl;

    if (magicOK)
        sendOpenConnectionReplyOne(sock, clientAddr, protocol);
}