#pragma once
#include <vector>
#include <cstdint>
using namespace std;

struct Frame {
    uint8_t Reliability;
    bool isSplit;
    
    uint32_t reliableIndex;
    uint32_t sequencingIndex;
    uint32_t orderingIndex;
    
    uint8_t orderingChannel;
    
    uint32_t splitCount;
    uint16_t splitId;
    uint32_t splitIndex;
    
    vector<uint8_t> buffer;
};

struct FrameSetPacket {
    uint8_t flags;
    uint32_t sequenceNumber;
    vector<Frame> frames;
};

class BinaryReader {
public: 
    const uint8_t* data;
    size_t length;
    size_t pos;
    
    BinaryReader(const uint8_t* data,size_t length)
        : data(data),length(length),pos(0) {}
    
    uint8_t readUint8() {
        return data[pos++];
    }
    
    uint16_t readInt16() {
        uint16_t value = (data[pos] << 8) | data[pos + 1];
        pos += 2;
        return value;
    }
    
    uint32_t readInt24LE() {
        uint32_t value = data[pos] | (data[pos +  1] << 8) | (data[pos + 2] << 16);
        pos += 3;
        return value;
    }
    
    uint32_t readInt32BE() {
        uint32_t value = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;
        return value;
    }
    
    vector<uint8_t> readBytes(size_t n) {
        vector<uint8_t> bytes(data + pos,data + pos + n);
        pos += n;
        return bytes;
    }
    
    bool hasMore() {
        return pos < length;
    }
};

FrameSetPacket decodeFrameSetPacket(const uint8_t* data, size_t length);
void printFrameSetPacket(const FrameSetPacket& packet);
vector<uint8_t> encodeFrameSetPacket(const FrameSetPacket& packet);
void printEncodedFrameSetPacket(const FrameSetPacket& packet);