#include "FrameSetPacket.h"
#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

bool isReliable(uint8_t reliability) {
    // 2=Reliable, 3=ReliableOrdered, 4=ReliableSequenced,
    // 6=ReliableWithAckReceipt, 7=ReliableOrderedWithAckReceipt
    return reliability == 2 || reliability == 3 || reliability == 4 ||
           reliability == 6 || reliability == 7;
}

bool isSequenced(uint8_t reliability) {
    // 1=UnreliableSequenced, 4=ReliableSequenced
    return reliability == 1 || reliability == 4;
}

bool isOrdered(uint8_t reliability) {
    // 1=UnreliableSequenced, 3=ReliableOrdered,
    // 4=ReliableSequenced, 7=ReliableOrderedWithAckReceipt
    return reliability == 1 || reliability == 3 || reliability == 4 ||
           reliability == 7;
}

FrameSetPacket decodeFrameSetPacket(const uint8_t* data, size_t length) {
    BinaryReader reader(data, length);
    FrameSetPacket packet;

    packet.flags = reader.readUint8();
    packet.sequenceNumber = reader.readInt24LE();

    while (reader.hasMore()) {
        Frame frame = {};

        uint8_t frameFlags = reader.readUint8();
        frame.Reliability = (frameFlags >> 5) & 0x07;
        frame.isSplit = (frameFlags >> 4) & 0x01;

        uint16_t bitLength = reader.readInt16();
        size_t byteLength = (bitLength + 7) / 8;

        if (isReliable(frame.Reliability)) {
            frame.reliableIndex = reader.readInt24LE();
        }

        if (isSequenced(frame.Reliability)) {
            frame.sequencingIndex = reader.readInt24LE();
        }

        if (isOrdered(frame.Reliability)) {
            frame.orderingIndex = reader.readInt24LE();
            frame.orderingChannel = reader.readUint8();
        }

        if (frame.isSplit) {
            frame.splitCount = reader.readInt32BE();
            frame.splitId    = reader.readInt16();
            frame.splitIndex = reader.readInt32BE();
        }

        frame.buffer = reader.readBytes(byteLength);
        packet.frames.push_back(frame);
    }
    return packet;
}

void printFrameSetPacket(const FrameSetPacket& packet) {
    cout << "===========================================" << endl;
    cout << "FrameSetPacket" << endl;
    cout << "===========================================" << endl;
    cout << "Flags:           0x" << hex << (int)packet.flags << dec << endl;
    cout << "Sequence Number: " << packet.sequenceNumber << endl;
    cout << "Frame Count:     " << packet.frames.size() << endl;

    for (size_t i = 0; i < packet.frames.size(); i++) {
        const Frame& frame = packet.frames[i];

        cout << "-------------------------------------------" << endl;
        cout << "Frame [" << i << "]" << endl;
        cout << "-------------------------------------------" << endl;
        cout << "  Reliability:      " << (int)frame.Reliability << endl;
        cout << "  isSplit:          " << (frame.isSplit ? "true" : "false") << endl;

        if (isReliable(frame.Reliability))
            cout << "  Reliable Index:   " << frame.reliableIndex << endl;

        if (isSequenced(frame.Reliability))
            cout << "  Sequencing Index: " << frame.sequencingIndex << endl;

        if (isOrdered(frame.Reliability)) {
            cout << "  Ordering Index:   " << frame.orderingIndex << endl;
            cout << "  Ordering Channel: " << (int)frame.orderingChannel << endl;
        }

        if (frame.isSplit) {
            cout << "  Split Count:      " << frame.splitCount << endl;
            cout << "  Split ID:         " << frame.splitId << endl;
            cout << "  Split Index:      " << frame.splitIndex << endl;
        }

        cout << "  Buffer Size:      " << frame.buffer.size() << " bytes" << endl;
        cout << "  Buffer (hex):     ";
        for (uint8_t byte : frame.buffer)
            cout << hex << setw(2) << setfill('0') << (int)byte << " ";
        cout << dec << endl;
    }
    cout << "===========================================" << endl;
}

vector<uint8_t> encodeFrameSetPacket(const FrameSetPacket& packet) {
    vector<uint8_t> out;

    // Datagram header
    out.push_back(packet.flags);
    out.push_back(packet.sequenceNumber & 0xFF);
    out.push_back((packet.sequenceNumber >> 8) & 0xFF);
    out.push_back((packet.sequenceNumber >> 16) & 0xFF);

    for (const Frame& frame : packet.frames) {
        // reliability (3 bits) | isSplit (1 bit) | 4 zero bits
        uint8_t frameFlags = ((frame.Reliability & 0x07) << 5) |
                             (frame.isSplit ? 0x10 : 0x00);
        out.push_back(frameFlags);

        // Buffer size in bits, big endian
        uint16_t bitLength = (uint16_t)(frame.buffer.size() * 8);
        out.push_back((bitLength >> 8) & 0xFF);
        out.push_back(bitLength & 0xFF);

        // reliable capsule index (only if reliable)
        if (isReliable(frame.Reliability)) {
            out.push_back(frame.reliableIndex & 0xFF);
            out.push_back((frame.reliableIndex >> 8) & 0xFF);
            out.push_back((frame.reliableIndex >> 16) & 0xFF);
        }

        // sequenced capsule index (only if sequenced)
        if (isSequenced(frame.Reliability)) {
            out.push_back(frame.sequencingIndex & 0xFF);
            out.push_back((frame.sequencingIndex >> 8) & 0xFF);
            out.push_back((frame.sequencingIndex >> 16) & 0xFF);
        }

        // ordering index + channel (only if ordered)
        if (isOrdered(frame.Reliability)) {
            out.push_back(frame.orderingIndex & 0xFF);
            out.push_back((frame.orderingIndex >> 8) & 0xFF);
            out.push_back((frame.orderingIndex >> 16) & 0xFF);
            out.push_back(frame.orderingChannel);
        }

        // split packet info (only if split)
        if (frame.isSplit) {
            // size: uint32 big endian
            out.push_back((frame.splitCount >> 24) & 0xFF);
            out.push_back((frame.splitCount >> 16) & 0xFF);
            out.push_back((frame.splitCount >> 8) & 0xFF);
            out.push_back(frame.splitCount & 0xFF);
            // id: uint16 big endian
            out.push_back((frame.splitId >> 8) & 0xFF);
            out.push_back(frame.splitId & 0xFF);
            // index: uint32 big endian
            out.push_back((frame.splitIndex >> 24) & 0xFF);
            out.push_back((frame.splitIndex >> 16) & 0xFF);
            out.push_back((frame.splitIndex >> 8) & 0xFF);
            out.push_back(frame.splitIndex & 0xFF);
        }

        out.insert(out.end(), frame.buffer.begin(), frame.buffer.end());
    }
    return out;
}

void printEncodedFrameSetPacket(const FrameSetPacket& packet) {
    vector<uint8_t> encoded = encodeFrameSetPacket(packet);

    cout << "===========================================" << endl;
    cout << "FrameSetPacket (Encoded)" << endl;
    cout << "===========================================" << endl;
    cout << "Flags:           0x" << hex << (int)packet.flags << dec << endl;
    cout << "Sequence Number: " << packet.sequenceNumber << endl;
    cout << "Frame Count:     " << packet.frames.size() << endl;

    for (size_t i = 0; i < packet.frames.size(); i++) {
        const Frame& frame = packet.frames[i];

        cout << "-------------------------------------------" << endl;
        cout << "Frame [" << i << "]" << endl;
        cout << "-------------------------------------------" << endl;
        cout << "  Reliability:      " << (int)frame.Reliability << endl;
        cout << "  isSplit:          " << (frame.isSplit ? "true" : "false") << endl;

        if (isReliable(frame.Reliability))
            cout << "  Reliable Index:   " << frame.reliableIndex << endl;

        if (isSequenced(frame.Reliability))
            cout << "  Sequencing Index: " << frame.sequencingIndex << endl;

        if (isOrdered(frame.Reliability)) {
            cout << "  Ordering Index:   " << frame.orderingIndex << endl;
            cout << "  Ordering Channel: " << (int)frame.orderingChannel << endl;
        }

        if (frame.isSplit) {
            cout << "  Split Count:      " << frame.splitCount << endl;
            cout << "  Split ID:         " << frame.splitId << endl;
            cout << "  Split Index:      " << frame.splitIndex << endl;
        }

        cout << "  Buffer Size:      " << frame.buffer.size() << " bytes" << endl;
        cout << "  Buffer (hex):     ";
        for (uint8_t byte : frame.buffer)
            cout << hex << setw(2) << setfill('0') << (int)byte << " ";
        cout << dec << endl;
    }

    cout << "  Encoded Size:     " << encoded.size() << " bytes" << endl;
    cout << "===========================================" << endl;
}