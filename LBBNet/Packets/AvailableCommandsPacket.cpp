#include "AvailableCommandsPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendAvailableCommands(int sock, sockaddr_in clientAddr, ClientState& state) {
    vector<uint8_t> packet;
    writeVarInt(packet, 76);
    writeVarInt(packet, 0); // values_len
    writeVarInt(packet, 0); // enum_values
    writeVarInt(packet, 0); // chained_subcommand_values
    writeVarInt(packet, 0); // suffixes
    writeVarInt(packet, 0); // enums
    writeVarInt(packet, 0); // chained_subcommands
    writeVarInt(packet, 0); // command_data
    writeVarInt(packet, 0); // dynamic_enums
    writeVarInt(packet, 0); // enum_constraints

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent empty AvailableCommands\n";
}
