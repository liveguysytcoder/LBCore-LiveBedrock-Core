#include "PlayStatusPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendPlayStatus(int sock, sockaddr_in clientAddr, ClientState& state, PlayStatusType status) {
    // Gamepacket header (varint) for PlayStatus = 0x02, followed by a
    // single "Int (big-endian)" Status field (field index 0) — both
    // confirmed directly against Mojang's official PlayStatusPacket doc.
    vector<uint8_t> packet;
    writeVarInt(packet, 0x02);
    writeIntBE(packet, static_cast<int32_t>(status));

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent PlayStatus(" << static_cast<int32_t>(status) << ")\n";
}
