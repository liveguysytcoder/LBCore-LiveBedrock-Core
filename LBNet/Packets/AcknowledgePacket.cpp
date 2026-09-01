#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "AcknowledgePacket.h"

using namespace std;

void sendACKPacket(int sock, sockaddr_in clientAddr, ClientState& state, uint32_t seqNum) {
    if (state.ackedSeqs.count(seqNum)) return;
    state.ackedSeqs.insert(seqNum);

    if (state.ackedSeqs.size() > 1000)
        state.ackedSeqs.erase(state.ackedSeqs.begin());

    uint8_t ack[7];
    ack[0] = 0xC0;
    ack[1] = 0x00;
    ack[2] = 0x01;
    ack[3] = 0x01; // 1 = single sequence number (no min/max range follows)
    ack[4] = seqNum & 0xFF;
    ack[5] = (seqNum >> 8) & 0xFF;
    ack[6] = (seqNum >> 16) & 0xFF;

    sendto(sock, ack, 7, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
}

void checkAndSendNACK(int sock, sockaddr_in clientAddr, ClientState& state, uint32_t receivedSeq) {
    if (!state.hasReceivedAny) {
        // First datagram from this client: nothing to compare against yet.
        state.hasReceivedAny = true;
        state.expectedSeq = (receivedSeq + 1) & MAX_SEQ;
        return;
    }

    uint32_t gap = (receivedSeq - state.expectedSeq) & MAX_SEQ;
    if (gap > 0 && gap < 1000) {
        for (uint32_t i = 0; i < gap; i++) {
            uint32_t missing = (state.expectedSeq + i) & MAX_SEQ;
            if (!state.ackedSeqs.count(missing)) {
                sendNACK(sock, clientAddr, missing);
            }
        }
    }
    state.expectedSeq = (receivedSeq + 1) & MAX_SEQ;
}

void sendNACK(int sock, sockaddr_in clientAddr, uint32_t seqNum) {
    uint8_t nack[7];
    nack[0] = 0xA0;
    nack[1] = 0x00;
    nack[2] = 0x01;
    // BUG FIX: this used to be 0x00 ("a min/max range follows"), but only a
    // single 3-byte sequence number was ever written after it. A real RakNet
    // peer parsing that would read the seqNum bytes as the range's low value
    // and then read garbage as the high value, desyncing the rest of the
    // packet. We only ever report a single missing sequence number here, so
    // the flag must say so.
    nack[3] = 0x01;
    nack[4] = seqNum & 0xFF;
    nack[5] = (seqNum >> 8) & 0xFF;
    nack[6] = (seqNum >> 16) & 0xFF;

    sendto(sock, nack, 7, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
}
