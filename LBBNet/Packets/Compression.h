#pragma once
#include <vector>
#include <cstdint>

using namespace std;

// Compression algorithm identifiers, per the Bedrock Protocol wiki:
//   Zlib:          0x00
//   Snappy:        0x01
//   No Compression: 0xFF (single-packet), 0xFFFF (in NetworkSettings)
enum class CompressionAlgorithm : uint16_t {
    Zlib = 0x00,
    Snappy = 0x01,
    None = 0xFFFF
};

// Compresses `input` using zlib (deflate). Returns false on failure.
bool zlibCompress(const vector<uint8_t>& input, vector<uint8_t>& output, int level = 7);

// Decompresses a zlib (deflate) stream. Returns false on failure (corrupt
// data, or the output would exceed maxOutputSize — a basic guard against
// decompression-bomb style payloads from untrusted clients).
bool zlibDecompress(const vector<uint8_t>& input, vector<uint8_t>& output,
                     size_t maxOutputSize = 8 * 1024 * 1024);
