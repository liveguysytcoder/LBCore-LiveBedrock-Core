#include "GameRulesChangedPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendGameRulesChanged(int sock, sockaddr_in clientAddr, ClientState& state,
                           const string& name, bool editable, bool value) {
    vector<uint8_t> packet;
    writeVarInt(packet, 72);

    writeVarInt(packet, 1); // rules: array count = 1
    writeString(packet, name);
    writeBool(packet, editable);
    writeVarInt(packet, 1); // type = Bool
    writeBool(packet, value);

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent GameRulesChanged(" << name << "=" << (value ? "true" : "false") << ")\n";
}
