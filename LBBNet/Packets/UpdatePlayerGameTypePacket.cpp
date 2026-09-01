#include "UpdatePlayerGameTypePacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendUpdatePlayerGameType(int sock, sockaddr_in clientAddr, ClientState& state,
                               int32_t gamemode, int64_t playerUniqueId, uint64_t tick) {
    vector<uint8_t> packet;
    writeVarInt(packet, 151);
    writeZigZag32(packet, gamemode);
    writeZigZag64(packet, playerUniqueId);
    writeVarInt64(packet, tick);

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent UpdatePlayerGameType(" << gamemode << ")\n";
}
