#include "UpdateAbilitiesPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendUpdateAbilities(int sock, sockaddr_in clientAddr, ClientState& state) {
    vector<uint8_t> packet;
    writeVarInt(packet, 187); // UpdateAbilities (0xBB) -- per PocketMine-MP's
                               // current ProtocolInfo.php.

    writeZigZag64(packet, 1); // EntityUniqueID -- this project's one fixed player id
    writeUByte(packet, 1);    // PlayerPermissions: Member (matches reference's "member")
    writeUByte(packet, 0);    // CommandPermissions: Normal (matches reference's "normal")

    writeUByte(packet, 1);       // AbilityLayers: array count = 1
    writeUShort(packet, 0);      // Layer Type = Base (0)
    writeUInt(packet, 0xFFFFF);  // Allowed abilities bitset -- reference capture's
                                  // combined raw value for a creative player
                                  // (all ~20 ability bits set true)
    writeUInt(packet, 0xFFFFF);  // "Values" bitset -- best-effort duplicate of
                                  // Allowed; UNVERIFIED, see header comment
    writeFloat(packet, 0.05f);   // FlySpeed (vanilla default)
    writeFloat(packet, 0.1f);    // WalkSpeed (vanilla default)

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent UpdateAbilities\n";
}
