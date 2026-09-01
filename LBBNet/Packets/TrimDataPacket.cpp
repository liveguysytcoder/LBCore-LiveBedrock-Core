#include "TrimDataPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendEmptyTrimData(int sock, sockaddr_in clientAddr, ClientState& state) {
    vector<uint8_t> packet;
    writeVarInt(packet, 302);
    writeVarInt(packet, 0); // patterns: array count = 0
    writeVarInt(packet, 0); // materials: array count = 0

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent empty TrimData\n";
}
