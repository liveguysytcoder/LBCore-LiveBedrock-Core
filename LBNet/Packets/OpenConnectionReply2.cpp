#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "RaknetGlobals.h"

using namespace std;

// Send Open Connection Reply 2 (0x08)
void sendOpenConnectionReplyTwo(int sock, sockaddr_in clientAddr, unsigned short mtu)
{
    vector<unsigned char> reply;

    reply.push_back(0x08); // Packet ID

    // RakNet magic
    reply.insert(reply.end(), RAKNET_MAGIC, RAKNET_MAGIC + 16);

    // Server GUID
    for (int i = 7; i >= 0; i--)
        reply.push_back((SERVER_GUID >> (i * 8)) & 0xFF);

    // Client address
    reply.push_back(4); // IPv4

    unsigned char* ipBytes = (unsigned char*)&clientAddr.sin_addr.s_addr;
    reply.push_back(ipBytes[0]);
    reply.push_back(ipBytes[1]);
    reply.push_back(ipBytes[2]);
    reply.push_back(ipBytes[3]);

    unsigned short port = ntohs(clientAddr.sin_port);
    reply.push_back((port >> 8) & 0xFF);
    reply.push_back(port & 0xFF);

    // MTU
    reply.push_back((mtu >> 8) & 0xFF);
    reply.push_back(mtu & 0xFF);

    // security disabled
    reply.push_back(0x00);

    sendto(sock, reply.data(), reply.size(), 0,
           (struct sockaddr*)&clientAddr, sizeof(clientAddr));

    //cout << "📤 Sent Open Connection Reply 2 (" << reply.size() << " bytes)\n";
}
