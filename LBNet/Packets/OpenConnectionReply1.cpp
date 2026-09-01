#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "RaknetGlobals.h"

using namespace std;

// Send Open Connection Reply 1 (0x06)
void sendOpenConnectionReplyOne(int sock, sockaddr_in clientAddr, unsigned char protocol) {

    vector<unsigned char> reply;

    reply.push_back(0x06);

    // RakNet magic
    reply.insert(reply.end(), RAKNET_MAGIC, RAKNET_MAGIC + 16);

    // Server GUID
    for (int i = 7; i >= 0; --i)
        reply.push_back((SERVER_GUID >> (i * 8)) & 0xFF);

    reply.push_back(0x00); // security disabled

    unsigned short mtu = 1492;
    reply.push_back((mtu >> 8) & 0xFF);
    reply.push_back(mtu & 0xFF);

    sendto(sock, reply.data(), reply.size(), 0,
           (struct sockaddr*)&clientAddr, sizeof(clientAddr));

    //cout << "📤 Sent Open Connection Reply 1 (" << reply.size() << " bytes)\n";
}
