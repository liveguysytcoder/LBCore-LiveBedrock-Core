#include "InventoryContentPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendInventoryContent(int sock, sockaddr_in clientAddr, ClientState& state, int32_t windowId) {
    // Gamepacket header (varint) for InventoryContent = 49 (0x31).
    vector<uint8_t> packet;
    writeVarInt(packet, 49);

    writeVarInt(packet, (uint32_t)windowId); // Container Id
    writeVarInt(packet, 0);                  // Slots: array count = 0 -- empty container

    // FIXED: confirmed against Mojang's official protocol docs (protocol
    // 2168 / 1.26.44) -- this packet actually has 4 top-level fields, not
    // 2. It was missing the whole FullContainerName struct and the
    // trailing StorageItem field entirely, so a real/js client parsing
    // past the (empty) Slots array immediately ran off the end of the
    // packet looking for FullContainerName's enum byte -- exactly the
    // "ContainerSlotType"/u8 partial-read error seen in testing.
    //
    // FullContainerName { ContainerEnumName as uint8, DynamicID as uint32 }
    //
    // IMPORTANT: this project's pseudo-window IDs (InventoryWindowId::*,
    // in the header: Inventory=0, OffHand=119, Armor=120) are NOT the same
    // numbering as the protocol's ContainerEnumName enum (InventoryContainer
    // =32, OffhandContainer=37, ArmorContainer=6, ...) -- casting windowId
    // directly into this byte would silently send the wrong container name
    // (e.g. windowId 0 would be misread as AnvilInputContainer instead of
    // InventoryContainer). Mapping the specific IDs this project currently
    // sends; extend this switch if InventoryWindowId grows.
    uint8_t containerEnumName;
    switch (windowId) {
        case InventoryWindowId::Inventory: containerEnumName = 32; break; // InventoryContainer
        case InventoryWindowId::OffHand:   containerEnumName = 37; break; // OffhandContainer
        case InventoryWindowId::Armor:     containerEnumName = 6;  break; // ArmorContainer
        default:                          containerEnumName = 32; break; // fall back to InventoryContainer
    }
    // CONFIRMED WRONG and fixed: DynamicID was written as a fixed 4-byte
    // uint32. Every other numeric identifier field in this protocol
    // (window/container/slot ids, array counts, etc.) is VarInt-encoded --
    // this project's own DataTypeHelper convention already reflects that --
    // and pmmp/BedrockProtocol's own field for this (added as a plain PHP
    // `int`, decoded the same VarInt way as every sibling id field in that
    // library) agrees. Writing 4 raw bytes for a value that should take 1
    // byte (0) left 3 stray bytes in the packet: a real/JS client reads
    // DynamicID as a 1-byte VarInt(0), then misreads the next 3 leftover
    // zero bytes as the start of the trailing StorageItem field, running
    // past the end of this (very short) sub-packet while trying to parse
    // an item descriptor that isn't really there -- exactly the
    // "Unexpected buffer end while reading VarInt" seen in a real capture
    // for this exact packet (InventoryContent).
    writeUByte(packet, containerEnumName); // FullContainerName.ContainerName
    writeVarInt(packet, 0);                // FullContainerName.DynamicID -- only used for DynamicContainer (66)

    // StorageItem (NetworkItemStackDescriptor) -- networkId 0 is the
    // standard "no item / air" encoding used across every item-descriptor
    // field in the protocol; no further bytes follow when it's 0.
    writeVarInt(packet, 0); // StorageItem: networkId = 0 (empty)

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent InventoryContent(window=" << windowId << ")\n";
}
