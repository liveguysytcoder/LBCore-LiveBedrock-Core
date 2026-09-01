#include "BlockHash.h"
#include <algorithm>

using namespace std;

// Classic little-endian NBT string: u16 LE length, then raw bytes.
static void writeNbtString(vector<uint8_t>& buf, const string& s) {
    uint16_t len = static_cast<uint16_t>(s.size());
    buf.push_back(len & 0xFF);
    buf.push_back((len >> 8) & 0xFF);
    buf.insert(buf.end(), s.begin(), s.end());
}

static uint32_t fnv1a32(const vector<uint8_t>& data) {
    uint32_t hash = 0x811c9dc5u; // FNV offset basis
    for (uint8_t b : data) {
        hash ^= b;
        hash *= 0x01000193u; // FNV prime
    }
    return hash;
}

int32_t computeBlockHash(const string& name, const vector<pair<string, BlockStateValue>>& states) {
    if (name == "minecraft:unknown") return -2; // special case, per the reference implementation

    vector<pair<string, BlockStateValue>> sorted = states;
    sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    vector<uint8_t> buf;
    buf.push_back(0x0A); // outer TAG_Compound
    buf.push_back(0x00);
    buf.push_back(0x00); // root name length = 0 (unnamed root tag)

    // "name": TAG_String
    buf.push_back(0x08);
    writeNbtString(buf, "name");
    writeNbtString(buf, name);

    // "states": TAG_Compound
    buf.push_back(0x0A);
    writeNbtString(buf, "states");
    for (auto& entry : sorted) {
        const string& key = entry.first;
        const BlockStateValue& value = entry.second;
        switch (value.type) {
            case BlockStateValue::Type::Byte:
                buf.push_back(0x01);
                writeNbtString(buf, key);
                buf.push_back(static_cast<uint8_t>(static_cast<int8_t>(value.intValue)));
                break;
            case BlockStateValue::Type::Int: {
                buf.push_back(0x03);
                writeNbtString(buf, key);
                uint32_t v = static_cast<uint32_t>(static_cast<int32_t>(value.intValue));
                buf.push_back(v & 0xFF);
                buf.push_back((v >> 8) & 0xFF);
                buf.push_back((v >> 16) & 0xFF);
                buf.push_back((v >> 24) & 0xFF);
                break;
            }
            case BlockStateValue::Type::String:
                buf.push_back(0x08);
                writeNbtString(buf, key);
                writeNbtString(buf, value.stringValue);
                break;
        }
    }
    buf.push_back(0x00); // TAG_End closes "states"
    buf.push_back(0x00); // TAG_End closes the outer compound

    return static_cast<int32_t>(fnv1a32(buf));
}
