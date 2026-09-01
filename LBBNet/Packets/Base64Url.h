#pragma once
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

// Decodes a base64url string (RFC 4648 §5 — '-'/'_' alphabet, padding
// optional), which is the encoding used for each dot-separated segment of
// the JWTs found in the Login packet's certificate chain and client data.
// Unknown/invalid characters (padding, stray whitespace) are skipped rather
// than treated as a hard error, since JWT segments in the wild are rarely
// padded consistently.
vector<uint8_t> base64UrlDecode(const string& input);

// Encodes bytes as base64url (RFC 4648 §5), no padding — used to build the
// header/payload/signature segments of the ServerToClientHandshake JWT we
// sign ourselves (see Encryption.h).
string base64UrlEncode(const vector<uint8_t>& data);

// Encodes bytes as standard base64 (RFC 4648 §4 — '+'/'/' alphabet, '='
// padding). The handshake JWT's "x5u" (EC public key, X.509 DER) and
// "salt" fields use plain base64, not the URL-safe variant JWT segments
// themselves use.
string base64Encode(const vector<uint8_t>& data);

// Decodes standard base64 (RFC 4648 §4) — used to turn the client's
// "identityPublicKey" claim (extracted from the login JWT payload) back
// into DER bytes for ECDH.
vector<uint8_t> base64Decode(const string& input);
