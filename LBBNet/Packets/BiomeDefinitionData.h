#pragma once
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

// One entry from the real (non-NBT) BiomeDefinitionList format used since
// the 1.19.70-ish "creative menu"/biome rework -- see
// BiomeDefinitionListPacket.cpp for the full field-order/type writeup and
// BiomeDefinitionData.cpp for where this data actually came from (a real
// server capture, not reconstructed).
struct BiomeDefEntry {
    uint32_t nameIndex;       // index into kBiomeStringList -- this entry's own name
    uint16_t biomeId;         // legacy id; 65535 (sentinel/unused) in every real entry seen
    float temperature;
    float downfall;
    float snowFoliage;
    float depth;
    float scale;
    int32_t mapWaterColour;   // packed ARGB, cosmetic (minimap tint) only
    bool rain;
    vector<uint32_t> tags;    // each value indexes into kBiomeStringList too
};

extern const vector<BiomeDefEntry> kBiomeDefinitions; // 87 entries, real capture data
extern const vector<string> kBiomeStringList;         // 146 entries, shared by name_index and tags[]

// biome_definitions[kPlainsBiomeArrayIndex] is "minecraft:plains" -- the
// only biome this project's chunks reference. See LevelChunkPacket.cpp's
// appendBiomeSection() for the (separate, still-unconfirmed) question of
// whether chunk data should reference biomes by this array position or by
// the old legacy numeric id.
constexpr uint32_t kPlainsBiomeArrayIndex = 49;
