#include <iostream>
#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ConnectionRequest.h"
#include "ConnectionRequestAccepted.h"

using namespace std;

ConnectionRequest decodeConnectionRequest(int sock, const uint8_t* data, size_t length, sockaddr_in clientAddr, ClientState& state) {
    ConnectionRequest packet = {};
    size_t offset = 0;

    packet.id = data[offset++];

    packet.clientGuid = 0;
    for (int i = 0; i < 8; i++)
        packet.clientGuid = (packet.clientGuid << 8) | data[offset++];

    packet.incomingTimeStamp = 0;
    for (int i = 0; i < 8; i++)
        packet.incomingTimeStamp = (packet.incomingTimeStamp << 8) | data[offset++];

    packet.doSecurity = data[offset++];

    if (packet.doSecurity) {
        for (int i = 0; i < 32; i++)
            packet.clientProof[i] = data[offset++];

        packet.doIdentity = data[offset++];

        if (packet.doIdentity) {
            for (int i = 0; i < 294; i++)
                packet.identityProof[i] = data[offset++];
        }
    }
    sendConnectionRequestAccepted(sock, clientAddr, state, packet.incomingTimeStamp);
    return packet;
}
