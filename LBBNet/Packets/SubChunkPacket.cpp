#include "SubChunkPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include "BlockHash.h"
#include <iostream>

using namespace std;

// BEST-EFFORT, NOT YET CONFIRMED AGAINST A REAL CAPTURE (unlike every other
// packet in this project, which was verified byte-for-byte against an
// actual client log). SubChunkRequest/SubChunk are among the
// least-documented Bedrock packets. What IS confirmed, directly from
// Mojang's own bedrock-protocol-docs repo (additional_docs/SubChunk Request
// System v1.18.10.md), covers the high-level shape:
//   - SubChunkPacket carries: a result code (1=Success, 2=chunk doesn't
//     exist, 3=wrong dimension, 5=index out of bounds), a serialized data
//     blob (sub-chunk terrain + BlockActor NBT combined), and a heightmap
//     (16x16 int8 in [z][x] order, with a status byte: 0=NoData,
//     1=HasData, 2=AllTooHigh, 3=AllTooLow).
//   - The request carries a dimension, a base sub-chunk position, and a
//     list of relative Y offsets.
// What ISN'T explicitly spelled out there (exact field order, whether
// offsets are 3 signed bytes each, whether Position is 2 or 3 components)
// is filled in here with the most common convention used by existing
// third-party implementations. If terrain still doesn't render after this,
// the next real-client capture showing the actual outgoing SubChunkRequest
// bytes (and however the client reacts to this response) is what's needed
// to correct any remaining field-order mismatch here -- same as every
// other packet in this project was ultimately nailed down.
namespace {

// Same construction as LevelChunkPacket.cpp's appendSuperflatSubchunk(),
// duplicated here (not shared) to keep this new, less-certain packet from
// touching that already-confirmed-correct code path.
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
// containing nothing but air -- for every requested offset outside our one
// populated layer.
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
    writeVarInt(buf, 1); // biome id 1 = plains
}

} // namespace

void sendSubChunkResponse(int sock, sockaddr_in clientAddr, ClientState& state,
                           int32_t dimension, int32_t baseX, int32_t baseY, int32_t baseZ,
                           const vector<SubChunkOffsetRequest>& offsets) {
    vector<uint8_t> packet;
    writeVarInt(packet, 161); // SubChunk

    writeBool(packet, false); // CacheEnabled -- skip the blob-cache path entirely, same as LevelChunk
    writeZigZag32(packet, dimension);
    writeZigZag32(packet, baseX);
    writeZigZag32(packet, baseY);
    writeZigZag32(packet, baseZ);

    writeVarInt(packet, (uint32_t)offsets.size());
    for (const auto& off : offsets) {
        writeByte(packet, off.dx);
        writeByte(packet, off.dy);
        writeByte(packet, off.dz);

        writeUByte(packet, 1); // Result: 1 = Success

        // HeightMapType: 0 = NoData -- no heightmap bytes follow. Simpler
        // and safer than computing a real one; worst case the client falls
        // back to its own default lighting/heightmap behavior for this
        // column instead of using ours.
        writeUByte(packet, 0);

        vector<uint8_t> subPayload;
        // absolute sub-chunk index 0 (world Y 0-15) is the only one that
        // actually has our superflat terrain; every other requested
        // offset is empty air. baseY is expected to be 0 for a
        // column-level request, so off.dy alone is treated as the
        // absolute sub-chunk index.
        if (off.dy == 0) {
            appendSuperflatSubchunk(subPayload);
        } else {
            appendAllAirSubchunk(subPayload);
        }
        appendBiomeSection(subPayload);
        // No block entities (nothing in this world has one) appended
        // after the block+biome data, matching LevelChunkPacket.cpp's own
        // reasoning for why zero is correct here.

        writeVarInt(packet, (uint32_t)subPayload.size());
        packet.insert(packet.end(), subPayload.begin(), subPayload.end());
    }

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent SubChunk response (" << offsets.size() << " offset(s), base=("
         << baseX << "," << baseY << "," << baseZ << "))\n";
}
