#include "SetEntityDataPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendSetEntityData(int sock, sockaddr_in clientAddr, ClientState& state) {
    vector<uint8_t> packet;
    writeVarInt(packet, 39); // SetActorData/SetEntityData (0x27) -- per
                              // PocketMine-MP's current ProtocolInfo.php.

    writeVarInt64(packet, 1); // Runtime Entity ID -- this project's one fixed player id
    writeVarInt(packet, 0);   // Metadata: array count = 0 (see header comment)
    writeVarInt(packet, 0);   // PropertiesInt: array count = 0
    writeVarInt(packet, 0);   // PropertiesFloat: array count = 0
    writeVarInt64(packet, 0); // Tick

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent SetEntityData (empty metadata)\n";
}
