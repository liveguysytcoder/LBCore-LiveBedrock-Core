#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../Server/Server.h"
#include "FrameSetPacket.h"
#include "ConnectionRequestAccepted.h"

using namespace std;

void sendConnectionRequestAccepted(int sock, sockaddr_in clientAddr, ClientState& state, uint64_t sendPingTime) {

    vector<unsigned char> reply;

    reply.push_back(0x10);
    reply.push_back(4);
    unsigned char* clientIp = (unsigned char*)&clientAddr.sin_addr.s_addr;
    reply.push_back(clientIp[0]);
    reply.push_back(clientIp[1]);
    reply.push_back(clientIp[2]);
    reply.push_back(clientIp[3]);
    unsigned short clientPort = ntohs(clientAddr.sin_port);
    reply.push_back((clientPort >> 8) & 0xFF);
    reply.push_back(clientPort & 0xFF);

    reply.push_back((state.index >> 8) & 0xFF);
    reply.push_back(state.index & 0xFF);

    sockaddr_in serverAddr = getServerAddress();
    unsigned char* serverIp = (unsigned char*)&serverAddr.sin_addr.s_addr;
    unsigned short serverPort = ntohs(serverAddr.sin_port);

    reply.push_back(4);
    reply.push_back(serverIp[0]);
    reply.push_back(serverIp[1]);
    reply.push_back(serverIp[2]);
    reply.push_back(serverIp[3]);
    reply.push_back((serverPort >> 8) & 0xFF);
    reply.push_back(serverPort & 0xFF);

    // Only 9 more here — the server address above already fills internal ID slot #0,
    // for a total of exactly 10 internal system addresses as the protocol requires.
    for (int i = 0; i < 9; i++) {
        reply.push_back(4);
        reply.push_back(0); reply.push_back(0);
        reply.push_back(0); reply.push_back(0);
        reply.push_back(0); reply.push_back(0);
    }
    for (int i = 7; i >= 0; i--)
        reply.push_back((sendPingTime >> (i * 8)) & 0xFF);

    uint64_t sendPongTime = sendPingTime + 1000;
    for (int i = 7; i >= 0; i--)
        reply.push_back((sendPongTime >> (i * 8)) & 0xFF);

    Frame frame;
    frame.Reliability     = 2;
    frame.isSplit         = false;
    frame.reliableIndex   = state.nextReliableIndex++;
    frame.orderingIndex   = 0;   // Reliability 2 is not ordered — never consumed by the encoder,
    frame.orderingChannel = 0;   // so don't burn a slot out of the real ordering-channel counter.
    frame.buffer          = reply;

    FrameSetPacket wrapper;
    wrapper.flags          = 0x84;
    wrapper.sequenceNumber = state.nextSeqNum++;
    wrapper.frames.push_back(frame);

    vector<uint8_t> encoded = encodeFrameSetPacket(wrapper);
    state.sentPackets[wrapper.sequenceNumber] = encoded;

    sendto(sock, encoded.data(), encoded.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));

    cout << "Sent ConnectionRequestAccepted(" << encoded.size() << " bytes)\n";
}
