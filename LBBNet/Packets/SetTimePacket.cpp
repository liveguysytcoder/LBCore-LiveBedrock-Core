#include "SetTimePacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendSetTime(int sock, sockaddr_in clientAddr, ClientState& state, int32_t time) {
    vector<uint8_t> packet;
    writeVarInt(packet, 10);
    writeZigZag32(packet, time);

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent SetTime(" << time << ")\n";
}
