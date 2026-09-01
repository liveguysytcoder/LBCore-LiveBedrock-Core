#include "Compression.h"
#include <zlib.h>
#include <iostream>

using namespace std;

// This file used to be an empty stub even though the Bedrock Protocol wiki
// documents Zlib as the compression algorithm to use in production. Any
// packet sent after NetworkSettings negotiates a real compression algorithm
// (instead of "None") needs these to actually work.

bool zlibCompress(const vector<uint8_t>& input, vector<uint8_t>& output, int level) {
    if (input.empty()) {
        output.clear();
        return true;
    }

    // Bedrock uses raw DEFLATE, not the standard zlib-wrapped stream (see
    // the wiki: "It uses the zlib RAW (also known as DEFLATE) format").
    // compress2()/inflateInit() assume a real zlib header+trailer, which a
    // real client never sends -- deflateInit2()/inflateInit2() with a
    // NEGATIVE windowBits (-15) is zlib's documented way to get headerless
    // raw deflate instead. Discovered via the packet logger: LBCore's own
    // NetworkSettings→Login round trip reassembled a 25931-byte batch fine,
    // but inflate() rejected it with code -3 (invalid header) because this
    // function was expecting bytes that don't exist in a raw stream.
    z_stream stream{};
    if (deflateInit2(&stream, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        cout << "[Compression] deflateInit2 (raw) failed\n";
        return false;
    }

    uLongf boundSize = deflateBound(&stream, (uLong)input.size());
    output.resize(boundSize);

    stream.next_in   = const_cast<Bytef*>(input.data());
    stream.avail_in  = (uInt)input.size();
    stream.next_out  = output.data();
    stream.avail_out = (uInt)output.size();

    int result = deflate(&stream, Z_FINISH);
    size_t produced = output.size() - stream.avail_out;
    deflateEnd(&stream);

    if (result != Z_STREAM_END) {
        cout << "[Compression] raw deflate failed with code " << result << "\n";
        output.clear();
        return false;
    }

    output.resize(produced);
    return true;
}

bool zlibDecompress(const vector<uint8_t>& input, vector<uint8_t>& output, size_t maxOutputSize) {
    if (input.empty()) {
        output.clear();
        return true;
    }

    // Matching raw-deflate fix as zlibCompress() above -- inflateInit2()
    // with windowBits=-15 for headerless raw deflate, instead of
    // inflateInit()'s default which expects a real zlib header/trailer.
    z_stream stream{};
    if (inflateInit2(&stream, -15) != Z_OK) {
        cout << "[Compression] inflateInit2 (raw) failed\n";
        return false;
    }

    stream.next_in = const_cast<Bytef*>(input.data());
    stream.avail_in = (uInt)input.size();

    output.clear();
    vector<uint8_t> chunk(16 * 1024);
    int ret;

    do {
        stream.next_out = chunk.data();
        stream.avail_out = (uInt)chunk.size();

        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            cout << "[Compression] zlib inflate failed with code " << ret << "\n";
            inflateEnd(&stream);
            output.clear();
            return false;
        }

        size_t produced = chunk.size() - stream.avail_out;
        if (output.size() + produced > maxOutputSize) {
            cout << "[Compression] Decompressed data exceeds max allowed size, aborting\n";
            inflateEnd(&stream);
            output.clear();
            return false;
        }
        output.insert(output.end(), chunk.begin(), chunk.begin() + produced);

    } while (ret != Z_STREAM_END);

    inflateEnd(&stream);
    return true;
}
