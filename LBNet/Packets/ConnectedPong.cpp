#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include "FrameSetPacket.h"
#include "ConnectedPong.h"

using namespace std;

void sendConnectedPong(int sock, sockaddr_in clientAddr, ClientState& state, uint64_t pingTime) {
    vector<uint8_t> reply;

    reply.push_back(0x03);
    for (int i = 7; i >= 0; i--)
        reply.push_back((pingTime >> (i * 8)) & 0xFF);

    uint64_t pongTime = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
    for (int i = 7; i >= 0; i--)
        reply.push_back((pongTime >> (i * 8)) & 0xFF);

    Frame frame;
    frame.Reliability = 0;   // Unreliable — ping/pong doesn't need reliability
    frame.isSplit = false;
    frame.buffer = reply;

    FrameSetPacket wrapper;
    wrapper.flags = 0x84;
    wrapper.sequenceNumber = state.nextSeqNum++;
    wrapper.frames.push_back(frame);

    vector<uint8_t> encoded = encodeFrameSetPacket(wrapper);
    state.sentPackets[wrapper.sequenceNumber] = encoded;
    sendto(sock, encoded.data(), encoded.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
    cout << "[RakNet] Sending ConnectedPong\n";
}
