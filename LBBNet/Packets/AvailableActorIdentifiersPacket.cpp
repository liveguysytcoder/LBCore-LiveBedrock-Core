#include "AvailableActorIdentifiersPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

namespace {

// Smallest valid NBT value for the CompoundTag this packet's single field
// expects — an empty TAG_Compound (type 0x0A, zero-length name, closed
// immediately by TAG_End). Same approach as StartGamePacket.cpp's
// writeEmptyNbtCompound(); duplicated locally rather than shared since
// there's no common NBT helper file yet.
void writeEmptyNbtCompound(vector<uint8_t>& buf) {
    // Same fix as StartGamePacket.cpp's writeEmptyNbtCompound() and
    // ItemRegistryData.cpp's per-item NBT: Network-Little-Endian NBT
    // length-prefixes tag names with a VarInt, not a 2-byte ushort. This
    // packet has only one field, so the old bug just left one stray
    // trailing byte in the sub-packet rather than corrupting later fields,
    // but it's still wrong wire format -- fixed here for consistency and
    // in case a stricter client validates exact sub-packet length.
    writeUByte(buf, 0x0A); // TAG_Compound
    writeVarInt(buf, 0);   // name length = 0, VarInt-encoded (1 byte)
    writeUByte(buf, 0x00); // TAG_End
}

} // namespace

void sendAvailableActorIdentifiers(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Gamepacket header (varint) for AvailableActorIdentifiers = 119 (0x77).
    vector<uint8_t> packet;
    writeVarInt(packet, 119);

    writeEmptyNbtCompound(packet); // Identifier List — no custom actor types to advertise

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent AvailableActorIdentifiers\n";
}
