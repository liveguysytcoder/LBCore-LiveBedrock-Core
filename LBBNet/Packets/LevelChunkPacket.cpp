#include "LevelChunkPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include "BlockHash.h"
#include <iostream>

using namespace std;

// REVERTED the SubChunkRequest/SubChunk skeleton rework -- confirmed wrong
// by capturing a real vanilla Bedrock Dedicated Server (Aternos, same game
// version, "vanilla software" per report) with a fixed packet-logger.js.
// Ground truth from that capture, across all 257 level_chunk packets it
// sent:
//   - EVERY one had sub_chunk_count = 0. Not a sentinel like 0xFFFFFFFF --
//     literally the value 0, every single time, including for chunks whose
//     payload clearly contains real embedded terrain data.
//   - A separate field, highest_subchunk_count, carried the real number of
//     sub-chunks (8-11 in the capture, varying with terrain height) --
//     THIS is what tells the client how many sub-chunk blobs to expect
//     inline in the payload.
//   - The client never once sent a subchunk_request packet in the entire
//     capture. Terrain arrives entirely inline in level_chunk's own
//     payload, just like the pre-1.18 format -- there's no
//     request/response round trip in practice for this version, despite
//     what the general SubChunk-Request-System doc describes elsewhere.
//   - A "dimension" field (varint, always 0 for overworld) sits between z
//     and sub_chunk_count -- a field this project's LevelChunk was
//     missing entirely even before the SubChunk detour.
// Field order confirmed from the capture's own consistent JSON key order:
// x, z, dimension, sub_chunk_count, highest_subchunk_count, cache_enabled,
// blobs (skipped, cache_enabled is always false), payload.
namespace {

void appendSuperflatSubchunk(vector<uint8_t>& buf) {
    buf.push_back(9); // sub-chunk version
    buf.push_back(1); // storage layer count

    const int bitsPerBlock = 2;
    const int blocksPerWord = 32 / bitsPerBlock; // 16
    const int totalBlocks = 16 * 16 * 16;         // 4096
    const int wordCount = (totalBlocks + blocksPerWord - 1) / blocksPerWord; // 256

    buf.push_back((uint8_t)((bitsPerBlock << 1) | 1)); // storage header

    vector<uint8_t> indices;
    indices.reserve(totalBlocks);
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = 0; y < 16; y++) {
                uint8_t paletteIndex;
                if (y == 0)       paletteIndex = 1; // bedrock
                else if (y <= 2)  paletteIndex = 2; // dirt
                else if (y == 3)  paletteIndex = 3; // grass
                else              paletteIndex = 0; // air
                indices.push_back(paletteIndex);
            }
        }
    }

    for (int w = 0; w < wordCount; w++) {
        uint32_t word = 0;
        for (int j = 0; j < blocksPerWord; j++) {
            int pos = w * blocksPerWord + j;
            uint32_t v = (pos < totalBlocks) ? indices[pos] : 0;
            word |= (v & ((1u << bitsPerBlock) - 1)) << (j * bitsPerBlock);
        }
        buf.push_back(word & 0xFF);
        buf.push_back((word >> 8) & 0xFF);
        buf.push_back((word >> 16) & 0xFF);
        buf.push_back((word >> 24) & 0xFF);
    }

    int32_t airHash     = computeBlockHash("minecraft:air", {});
    int32_t bedrockHash = computeBlockHash("minecraft:bedrock", {{"infiniburn_bit", BlockStateValue::ofInt(0)}});
    int32_t dirtHash     = computeBlockHash("minecraft:dirt", {});
    int32_t grassHash    = computeBlockHash("minecraft:grass_block", {});

    writeVarInt(buf, 4);
    writeZigZag32(buf, airHash);
    writeZigZag32(buf, bedrockHash);
    writeZigZag32(buf, dirtHash);
    writeZigZag32(buf, grassHash);
}

// A single storage-layer, single-value ("0 bits per block") sub-chunk
// containing nothing but air, for the sub-chunk layers above our one
// populated layer (world Y 16 and up, up to whatever height this column
// reports via highest_subchunk_count).
void appendAllAirSubchunk(vector<uint8_t>& buf) {
    buf.push_back(9); // sub-chunk version
    buf.push_back(1); // storage layer count
    buf.push_back((uint8_t)((0 << 1) | 1)); // storage header, 0 bits per block
    int32_t airHash = computeBlockHash("minecraft:air", {});
    writeVarInt(buf, 1);
    writeZigZag32(buf, airHash);
}

void appendBiomeSection(vector<uint8_t>& buf) {
    buf.push_back((uint8_t)((0 << 1) | 1)); // storage header, uniform value
    writeVarInt(buf, 1); // palette size
    // biome id 1 = plains (legacy numeric id, per pmmp/BedrockData's
    // biome_id_map.json). NOTE: BiomeDefinitionListPacket.cpp/
    // BiomeDefinitionData.cpp now send the real, non-NBT
    // BiomeDefinitionList, where every entry's own "biome_id" field is a
    // dead 65535 sentinel — meaning it's genuinely unclear whether this
    // per-chunk paletted storage is still supposed to reference the OLD
    // legacy numeric id (kept as-is here, unchanged) or the new
    // definition's ARRAY POSITION instead (kBiomeDefinitionData.h's
    // kPlainsBiomeArrayIndex = 49, if so). If chunks still don't render
    // after the BiomeDefinitionList format fix, this is the next thing to
    // try swapping.
    writeVarInt(buf, 1);
}

} // namespace

void sendLevelChunk(int sock, sockaddr_in clientAddr, ClientState& state, int32_t chunkX, int32_t chunkZ) {
    vector<uint8_t> packet;
    writeVarInt(packet, 58); // LevelChunk

    writeZigZag32(packet, chunkX);
    writeZigZag32(packet, chunkZ);
    writeVarInt(packet, 0);  // Dimension: 0 = overworld
    writeVarInt(packet, 0);  // sub_chunk_count -- confirmed always 0 in real captures
    // FIXED: official docs (mojang.github.io/bedrock-protocol-docs, packet
    // 58 field #3 "Client Request SubChunk Limit") declare this field as a
    // *signed* varint32, but this was writing it as an unsigned varint.
    // For the value 1 that's a real mismatch: raw unsigned-varint byte 0x01
    // decodes correctly as 1 either way, so this alone wasn't the
    // disconnect -- but it's still wrong per spec, so fixed here to avoid
    // corrupting this field for any chunk that ever needs a value where
    // signed/unsigned encoding actually diverges (e.g. -1 as a sentinel).
    writeZigZag32(packet, 1); // highest_subchunk_count / "Client Request SubChunk Limit" -- one 16-block layer (world Y 0-15)
    writeBool(packet, false); // CacheEnabled -- no blob-cache transaction
    // FIXED: field #5 "Cache Metadata" (array<SubChunkMetadata>) has no
    // "optional" tag in the official docs, unlike field #3 above -- meaning
    // it's an unconditional array that always needs its length prefix
    // written, even when empty. This was skipped entirely, which shifts
    // every byte of "Serialized Chunk Data" (field #6) after it by however
    // many bytes this missing length-varint should have been: the client
    // ends up parsing the chunk payload starting from the wrong offset,
    // producing garbage terrain it silently rejects. This is almost
    // certainly the actual cause of the mid-loading-screen disconnect --
    // the client accepted everything up through the last LevelChunk send,
    // then dropped the connection right after, consistent with a parse
    // failure on malformed chunk data rather than a timeout.
    writeVarInt(packet, 0);  // Cache Metadata: 0 entries (CacheEnabled is false)

    vector<uint8_t> rawPayload;
    appendSuperflatSubchunk(rawPayload); // the one real sub-chunk (world Y 0-15)
    appendBiomeSection(rawPayload);
    writeVarInt(rawPayload, 0); // Border blocks (Education Edition only) -- none
    // No block entities -- nothing in this world has one.

    writeVarInt(packet, (uint32_t)rawPayload.size());
    packet.insert(packet.end(), rawPayload.begin(), rawPayload.end());

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent LevelChunk(" << chunkX << ", " << chunkZ << ") -- inline, 1 sub-chunk\n";
}
