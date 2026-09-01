#include "SetPlayerGameTypePacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendSetPlayerGameType(int sock, sockaddr_in clientAddr, ClientState& state, int32_t gamemode) {
    vector<uint8_t> packet;
    writeVarInt(packet, 62);
    writeZigZag32(packet, gamemode); // 1 = creative, matches StartGame's player_gamemode

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent SetPlayerGameType(" << gamemode << ")\n";
}
