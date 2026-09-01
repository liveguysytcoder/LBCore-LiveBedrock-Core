#include "CreativeContentPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendCreativeContent(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Gamepacket header (varint) for CreativeContent = 145 (0x91).
    vector<uint8_t> packet;
    writeVarInt(packet, 145);

    writeVarInt(packet, 0); // Groups: array count = 0
    writeVarInt(packet, 0); // Entries: array count = 0

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent CreativeContent\n";
}
