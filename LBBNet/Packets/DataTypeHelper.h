#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

using namespace std;

// ─────────────────────────────────────────────
//  WRITE
// ─────────────────────────────────────────────

// Primitives (Little-Endian — Bedrock default)
void writeByte    (vector<uint8_t>& buf, int8_t value);
void writeUByte   (vector<uint8_t>& buf, uint8_t value);
void writeShort   (vector<uint8_t>& buf, int16_t value);
void writeUShort  (vector<uint8_t>& buf, uint16_t value);
void writeInt     (vector<uint8_t>& buf, int32_t value);
void writeIntBE   (vector<uint8_t>& buf, int32_t value); // Big-Endian (PlayStatus, protocol-version fields)
void writeUInt    (vector<uint8_t>& buf, uint32_t value);
void writeInt64   (vector<uint8_t>& buf, int64_t value);
void writeUInt64  (vector<uint8_t>& buf, uint64_t value);
void writeFloat   (vector<uint8_t>& buf, float value);
void writeDouble  (vector<uint8_t>& buf, double value);
void writeBool    (vector<uint8_t>& buf, bool value);
void writeUInt24LE(vector<uint8_t>& buf, uint32_t value); // 3-byte LE (RakNet seq nums)

// Variable-length integers (ZigZag for signed)
void writeVarInt  (vector<uint8_t>& buf, uint32_t value); // unsigned varint
void writeVarInt64(vector<uint8_t>& buf, uint64_t value); // unsigned varint64
void writeZigZag32(vector<uint8_t>& buf, int32_t value);  // signed varint
void writeZigZag64(vector<uint8_t>& buf, int64_t value);  // signed varint64

// String (unsigned varint length prefix + UTF-8 bytes)
void writeString  (vector<uint8_t>& buf, const string& value);

// ─────────────────────────────────────────────
//  READ  (advance offset in-place)
// ─────────────────────────────────────────────

int8_t   readByte    (const vector<uint8_t>& buf, size_t& offset);
uint8_t  readUByte   (const vector<uint8_t>& buf, size_t& offset);
int16_t  readShort   (const vector<uint8_t>& buf, size_t& offset);
uint16_t readUShort  (const vector<uint8_t>& buf, size_t& offset);
int32_t  readInt     (const vector<uint8_t>& buf, size_t& offset);
uint32_t readUInt    (const vector<uint8_t>& buf, size_t& offset);
int64_t  readInt64   (const vector<uint8_t>& buf, size_t& offset);
uint64_t readUInt64  (const vector<uint8_t>& buf, size_t& offset);
float    readFloat   (const vector<uint8_t>& buf, size_t& offset);
double   readDouble  (const vector<uint8_t>& buf, size_t& offset);
bool     readBool    (const vector<uint8_t>& buf, size_t& offset);
uint32_t readUIntBE(const vector<uint8_t>& buf, size_t& offset);
uint32_t readUInt24LE(const vector<uint8_t>& buf, size_t& offset);

uint32_t readVarInt  (const vector<uint8_t>& buf, size_t& offset);
uint64_t readVarInt64(const vector<uint8_t>& buf, size_t& offset);
int32_t  readZigZag32(const vector<uint8_t>& buf, size_t& offset);
int64_t  readZigZag64(const vector<uint8_t>& buf, size_t& offset);

string   readString  (const vector<uint8_t>& buf, size_t& offset);