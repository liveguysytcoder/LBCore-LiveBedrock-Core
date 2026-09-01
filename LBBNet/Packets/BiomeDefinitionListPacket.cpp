#include "BiomeDefinitionListPacket.h"
#include "BiomeDefinitionData.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendBiomeDefinitionList(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Gamepacket header (varint) = 122 (0x7A), same ID as before — only
    // the payload format changed. Per-entry field order/types below were
    // read directly off a real capture's JSON (protodef preserves
    // declared field order, so JSON key order == wire order):
    //   name_index        VarInt   -- index into string_list, this entry's own name
    //   biome_id          u16 LE   -- constant 65535 in every real entry; a dead
    //                                 legacy-id sentinel, not meaningfully used
    //   temperature       f32 LE
    //   downfall          f32 LE
    //   snow_foliage      f32 LE
    //   depth             f32 LE
    //   scale             f32 LE
    //   map_water_colour  i32 LE   -- packed ARGB, cosmetic only
    //   rain              bool (1 byte)
    //   tags              VarInt count, then VarInt[] -- each indexes string_list too
    // Top level: VarInt count + biome_definitions[], then VarInt count + string_list[]
    // (each string is the same length-prefixed String type used everywhere
    // else in this codebase).
    //
    // Confidence: HIGH on the array/field shapes (matches the reference
    // capture exactly) and on temperature/downfall/depth/scale/rain/tags
    // (plain float/bool/varint, unambiguous from the captured values).
    // biome_id as u16-not-varint and map_water_colour as i32-not-varint
    // are inferred (see BiomeDefinitionData.h's header comment) rather
    // than independently confirmed byte-for-byte -- if a real client
    // still rejects this packet, those two fields are the first things to
    // re-check.
    vector<uint8_t> packet;
    writeVarInt(packet, 122);

    writeVarInt(packet, (uint32_t)kBiomeDefinitions.size());
    for (const BiomeDefEntry& def : kBiomeDefinitions) {
        writeVarInt(packet, def.nameIndex);
        writeUShort(packet, def.biomeId);
        writeFloat(packet, def.temperature);
        writeFloat(packet, def.downfall);
        writeFloat(packet, def.snowFoliage);
        writeFloat(packet, def.depth);
        writeFloat(packet, def.scale);
        writeInt(packet, def.mapWaterColour);
        writeBool(packet, def.rain);
        writeVarInt(packet, (uint32_t)def.tags.size());
        for (uint32_t tag : def.tags) writeVarInt(packet, tag);
    }

    writeVarInt(packet, (uint32_t)kBiomeStringList.size());
    for (const string& s : kBiomeStringList) writeString(packet, s);

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent BiomeDefinitionList (" << kBiomeDefinitions.size()
         << " biomes, " << kBiomeStringList.size() << " strings)\n";
}

void sendEmptyBiomeDefinitionList(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Same gamepacket ID (122 / 0x7A), payload is just the two top-level
    // array counts, both zero — exactly what marshalling a zero-value
    // packet.BiomeDefinitionList{} produces in the new (non-NBT) format.
    vector<uint8_t> packet;
    writeVarInt(packet, 122);
    writeVarInt(packet, 0); // biome_definitions count
    writeVarInt(packet, 0); // string_list count

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent empty BiomeDefinitionList (pre-1.21.80 compat)\n";
}
