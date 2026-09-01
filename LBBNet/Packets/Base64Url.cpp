#include "Base64Url.h"

namespace {

int8_t decodeChar(char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<int8_t>(c - 'A');
    if (c >= 'a' && c <= 'z') return static_cast<int8_t>(c - 'a' + 26);
    if (c >= '0' && c <= '9') return static_cast<int8_t>(c - '0' + 52);
    if (c == '-') return 62; // base64url uses '-' where standard base64 uses '+'
    if (c == '_') return 63; // base64url uses '_' where standard base64 uses '/'
    return -1; // padding ('=') or anything else we don't recognize
}

const char kUrlAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
const char kStdAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int8_t decodeStdChar(char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<int8_t>(c - 'A');
    if (c >= 'a' && c <= 'z') return static_cast<int8_t>(c - 'a' + 26);
    if (c >= '0' && c <= '9') return static_cast<int8_t>(c - '0' + 52);
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1; // padding ('=') or anything else we don't recognize
}

// Shared 3-bytes-in/4-chars-out core for both alphabets; `padded` controls
// whether the standard '=' padding is appended for a trailing 1- or
// 2-byte group.
string encodeCore(const vector<uint8_t>& data, const char* alphabet, bool padded) {
    string out;
    size_t i = 0;
    while (i + 3 <= data.size()) {
        uint32_t n = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i + 1]) << 8) |
                     static_cast<uint32_t>(data[i + 2]);
        out += alphabet[(n >> 18) & 0x3F];
        out += alphabet[(n >> 12) & 0x3F];
        out += alphabet[(n >> 6) & 0x3F];
        out += alphabet[n & 0x3F];
        i += 3;
    }

    size_t rem = data.size() - i;
    if (rem == 1) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        out += alphabet[(n >> 18) & 0x3F];
        out += alphabet[(n >> 12) & 0x3F];
        if (padded) out += "==";
    } else if (rem == 2) {
        uint32_t n = (static_cast<uint32_t>(data[i]) << 16) | (static_cast<uint32_t>(data[i + 1]) << 8);
        out += alphabet[(n >> 18) & 0x3F];
        out += alphabet[(n >> 12) & 0x3F];
        out += alphabet[(n >> 6) & 0x3F];
        if (padded) out += "=";
    }
    return out;
}

} // namespace

vector<uint8_t> base64UrlDecode(const string& input) {
    vector<uint8_t> out;
    int buffer = 0;
    int bitsCollected = 0;

    for (char c : input) {
        int8_t value = decodeChar(c);
        if (value < 0) continue; // skip padding/whitespace instead of aborting

        buffer = (buffer << 6) | value;
        bitsCollected += 6;

        if (bitsCollected >= 8) {
            bitsCollected -= 8;
            out.push_back(static_cast<uint8_t>((buffer >> bitsCollected) & 0xFF));
        }
    }
    return out;
}

string base64UrlEncode(const vector<uint8_t>& data) {
    return encodeCore(data, kUrlAlphabet, /*padded=*/false);
}

string base64Encode(const vector<uint8_t>& data) {
    return encodeCore(data, kStdAlphabet, /*padded=*/true);
}

vector<uint8_t> base64Decode(const string& input) {
    vector<uint8_t> out;
    int buffer = 0;
    int bitsCollected = 0;

    for (char c : input) {
        int8_t value = decodeStdChar(c);
        if (value < 0) continue; // skip padding/whitespace instead of aborting

        buffer = (buffer << 6) | value;
        bitsCollected += 6;

        if (bitsCollected >= 8) {
            bitsCollected -= 8;
            out.push_back(static_cast<uint8_t>((buffer >> bitsCollected) & 0xFF));
        }
    }
    return out;
}
