#include "LevelEventPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendLevelEvent(int sock, sockaddr_in clientAddr, ClientState& state,
                     LevelEventType event, float x, float y, float z, int32_t data) {
    vector<uint8_t> packet;
    writeVarInt(packet, 25);
    writeZigZag32(packet, (int32_t)event);
    writeFloat(packet, x);
    writeFloat(packet, y);
    writeFloat(packet, z);
    writeZigZag32(packet, data);

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent LevelEvent(" << (int32_t)event << ")\n";
}
