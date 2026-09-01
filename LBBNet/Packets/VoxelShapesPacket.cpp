#include "VoxelShapesPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendVoxelShapes(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Gamepacket header (varint) for VoxelShapes = 337 (0x151).
    //
    // Field layout, per the protocol doc, in order:
    //   0: Shapes   (array<SerializableVoxelShape>) — each shape has a
    //      Cells grid (X/Y/Z size + solid/empty Storage bytes), plus
    //      X/Y/Z Coordinates arrays for the cell boundaries.
    //   1: Name Map (object<hashed_string, RegistryHandle>) — maps a
    //      voxel shape's name to its handle.
    //   2: Custom Shape Count (uint16)
    //
    // "array<T>" fields elsewhere in the Bedrock protocol (as opposed to
    // packets with their own hand-documented fixed layout, like
    // ResourcePacksStack's LE int32 pack-stack counts) are unsigned
    // varint-prefixed, so that's what's used for the Shapes array and the
    // Name Map object below. We don't define any custom voxel shapes, so
    // every collection here is empty — this is just the minimal valid
    // "no custom shapes" packet.
    vector<uint8_t> packet;
    writeVarInt(packet, 337);

    writeVarInt(packet, 0); // Shapes: array count = 0, no custom voxel shapes
    writeVarInt(packet, 0); // Name Map: entry count = 0
    writeUShort(packet, 0); // Custom Shape Count = 0

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent VoxelShapes\n";
}
