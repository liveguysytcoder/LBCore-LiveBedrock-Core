#include <iostream>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "RaknetGlobals.h"
#include "../Server/ServerManager.h"
#include "UnconnectedPong.h"

using namespace std;

void sendUnconnectedPong(int sock, sockaddr_in clientAddr, const vector<unsigned char>& pingData) {

    vector<unsigned char> pong;

    pong.push_back(0x1C);

    // timestamp
    for (int i = 1; i <= 8 && i < (int)pingData.size(); i++)
        pong.push_back(pingData[i]);

    // server GUID
    for (int i = 7; i >= 0; i--)
        pong.push_back((SERVER_GUID >> (i * 8)) & 0xFF);

    // RakNet magic
    pong.insert(pong.end(), RAKNET_MAGIC, RAKNET_MAGIC + 16);

    // MOTD
    string motd = getServerProperties();
    unsigned short motdLen = motd.size();

    pong.push_back((motdLen >> 8) & 0xFF);
    pong.push_back(motdLen & 0xFF);

    pong.insert(pong.end(), motd.begin(), motd.end());

    sendto(sock, pong.data(), pong.size(), 0,
           (struct sockaddr*)&clientAddr, sizeof(clientAddr));

    //cout << "Sent Unconnected Pong (" << pong.size() << " bytes)\n";
}
