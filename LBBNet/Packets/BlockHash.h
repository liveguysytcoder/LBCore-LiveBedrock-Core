#pragma once
#include <string>
#include <vector>
#include <cstdint>

using namespace std;

// A tiny closed set of NBT value types -- just enough to describe the
// handful of block states this project actually uses.
struct BlockStateValue {
    enum class Type { Byte, Int, String } type;
    int64_t intValue = 0;
    string stringValue;

    static BlockStateValue ofByte(int8_t v)   { BlockStateValue s; s.type = Type::Byte;   s.intValue = v; return s; }
    static BlockStateValue ofInt(int32_t v)   { BlockStateValue s; s.type = Type::Int;    s.intValue = v; return s; }
    static BlockStateValue ofString(string v) { BlockStateValue s; s.type = Type::String; s.stringValue = std::move(v); return s; }
};

// Computes the "hashed block network id" used when StartGamePacket's
// BlockNetworkIdsAreHashes flag is true: an FNV-1a 32-bit hash of the
// block's *classic* little-endian NBT encoding -- an unnamed root compound
// `{name: <block name>, states: {...sorted by key...}}` -- matching the
// reference implementation at
// https://gist.github.com/Alemiz112/504d0f79feac7ef57eda174b668dd345
// ("Example implementation of Minecraft: Bedrock block state network hash
// computation").
//
// This is the mechanism that lets a server send real (non-air) blocks
// without owning an exact, version-matched copy of the connecting
// client's compiled-in block-runtime-ID table -- the client re-derives the
// same hash from its own block registry and looks the block up by content
// rather than by a pre-agreed integer index. Note this uses CLASSIC LE NBT
// (u16-length-prefixed strings, plain 4-byte ints) for the hash input, NOT
// the varint-flavored "network NBT" used elsewhere in this protocol (e.g.
// writeEmptyNbtCompound) -- the two are different encodings.
int32_t computeBlockHash(const string& name, const vector<pair<string, BlockStateValue>>& states);
