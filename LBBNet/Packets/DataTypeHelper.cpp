#include "DataTypeHelper.h"

// ════════════════════════════════════════════
//  WRITE HELPERS
// ════════════════════════════════════════════

void writeByte(vector<uint8_t>& buf, int8_t value) {
    buf.push_back(static_cast<uint8_t>(value));
}

void writeUByte(vector<uint8_t>& buf, uint8_t value) {
    buf.push_back(value);
}

void writeBool(vector<uint8_t>& buf, bool value) {
    buf.push_back(value ? 0x01 : 0x00);
}

// Little-Endian (Bedrock default for fixed-width types)
void writeShort(vector<uint8_t>& buf, int16_t value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8) & 0xFF);
}

void writeUShort(vector<uint8_t>& buf, uint16_t value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8) & 0xFF);
}

void writeInt(vector<uint8_t>& buf, int32_t value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8)  & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
    buf.push_back((value >> 24) & 0xFF);
}

void writeIntBE(vector<uint8_t>& buf, int32_t value) {
    buf.push_back((value >> 24) & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
    buf.push_back((value >> 8)  & 0xFF);
    buf.push_back(value & 0xFF);
}

void writeUInt(vector<uint8_t>& buf, uint32_t value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8)  & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
    buf.push_back((value >> 24) & 0xFF);
}

void writeInt64(vector<uint8_t>& buf, int64_t value) {
    for (int i = 0; i < 8; i++)
        buf.push_back((value >> (i * 8)) & 0xFF);
}

void writeUInt64(vector<uint8_t>& buf, uint64_t value) {
    for (int i = 0; i < 8; i++)
        buf.push_back((value >> (i * 8)) & 0xFF);
}

void writeFloat(vector<uint8_t>& buf, float value) {
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    writeUInt(buf, raw);  // same LE byte order
}

void writeDouble(vector<uint8_t>& buf, double value) {
    uint64_t raw;
    memcpy(&raw, &value, sizeof(raw));
    writeUInt64(buf, raw);
}

// 3-byte Little-Endian (used for RakNet sequence numbers)
void writeUInt24LE(vector<uint8_t>& buf, uint32_t value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8)  & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
}

// Unsigned varint (up to 5 bytes)
void writeVarInt(vector<uint8_t>& buf, uint32_t value) {
    while (value >= 0x80) {
        buf.push_back((value & 0x7F) | 0x80);
        value >>= 7;
    }
    buf.push_back(value);
}

// Unsigned varint64 (up to 10 bytes)
void writeVarInt64(vector<uint8_t>& buf, uint64_t value) {
    while (value >= 0x80) {
        buf.push_back((value & 0x7F) | 0x80);
        value >>= 7;
    }
    buf.push_back(value);
}

// Signed varint32 via ZigZag encoding
void writeZigZag32(vector<uint8_t>& buf, int32_t value) {
    writeVarInt(buf, (static_cast<uint32_t>(value) << 1) ^ (value >> 31));
}

// Signed varint64 via ZigZag encoding
void writeZigZag64(vector<uint8_t>& buf, int64_t value) {
    writeVarInt64(buf, (static_cast<uint64_t>(value) << 1) ^ (value >> 63));
}

// String: unsigned varint length + raw UTF-8 bytes
void writeString(vector<uint8_t>& buf, const string& value) {
    writeVarInt(buf, static_cast<uint32_t>(value.size()));
    buf.insert(buf.end(), value.begin(), value.end());
}


// ════════════════════════════════════════════
//  READ HELPERS
// ════════════════════════════════════════════

int8_t readByte(const vector<uint8_t>& buf, size_t& offset) {
    return static_cast<int8_t>(buf[offset++]);
}

uint8_t readUByte(const vector<uint8_t>& buf, size_t& offset) {
    return buf[offset++];
}

bool readBool(const vector<uint8_t>& buf, size_t& offset) {
    return buf[offset++] != 0x00;
}

int16_t readShort(const vector<uint8_t>& buf, size_t& offset) {
    int16_t v = buf[offset] | (buf[offset + 1] << 8);
    offset += 2;
    return v;
}

uint16_t readUShort(const vector<uint8_t>& buf, size_t& offset) {
    uint16_t v = buf[offset] | (buf[offset + 1] << 8);
    offset += 2;
    return v;
}

int32_t readInt(const vector<uint8_t>& buf, size_t& offset) {
    int32_t v = buf[offset] | (buf[offset+1] << 8) | (buf[offset+2] << 16) | (buf[offset+3] << 24);
    offset += 4;
    return v;
}

uint32_t readUInt(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t v = buf[offset] | (buf[offset+1] << 8) | (buf[offset+2] << 16) | (buf[offset+3] << 24);
    offset += 4;
    return v;
}

int64_t readInt64(const vector<uint8_t>& buf, size_t& offset) {
    int64_t v = 0;
    for (int i = 0; i < 8; i++)
        v |= static_cast<int64_t>(buf[offset + i]) << (i * 8);
    offset += 8;
    return v;
}

uint64_t readUInt64(const vector<uint8_t>& buf, size_t& offset) {
    uint64_t v = 0;
    for (int i = 0; i < 8; i++)
        v |= static_cast<uint64_t>(buf[offset + i]) << (i * 8);
    offset += 8;
    return v;
}

float readFloat(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t raw = readUInt(buf, offset);
    float v;
    memcpy(&v, &raw, sizeof(v));
    return v;
}

double readDouble(const vector<uint8_t>& buf, size_t& offset) {
    uint64_t raw = readUInt64(buf, offset);
    double v;
    memcpy(&v, &raw, sizeof(v));
    return v;
}

uint32_t readUIntBE(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t v = (buf[offset] << 24) | (buf[offset+1] << 16)
               | (buf[offset+2] << 8) | buf[offset+3];
    offset += 4;
    return v;
}

uint32_t readUInt24LE(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t v = buf[offset] | (buf[offset+1] << 8) | (buf[offset+2] << 16);
    offset += 3;
    return v;
}

uint32_t readVarInt(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t result = 0;
    int shift = 0;
    while (offset < buf.size()) {
        uint8_t b = buf[offset++];
        result |= static_cast<uint32_t>(b & 0x7F) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }
    return result;
}

uint64_t readVarInt64(const vector<uint8_t>& buf, size_t& offset) {
    uint64_t result = 0;
    int shift = 0;
    while (offset < buf.size()) {
        uint8_t b = buf[offset++];
        result |= static_cast<uint64_t>(b & 0x7F) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }
    return result;
}

int32_t readZigZag32(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t raw = readVarInt(buf, offset);
    return static_cast<int32_t>((raw >> 1) ^ -(raw & 1));
}

int64_t readZigZag64(const vector<uint8_t>& buf, size_t& offset) {
    uint64_t raw = readVarInt64(buf, offset);
    return static_cast<int64_t>((raw >> 1) ^ -(static_cast<int64_t>(raw & 1)));
}

string readString(const vector<uint8_t>& buf, size_t& offset) {
    uint32_t len = readVarInt(buf, offset);
    string s(buf.begin() + offset, buf.begin() + offset + len);
    offset += len;
    return s;
}