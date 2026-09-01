#pragma once
#include <string>

using namespace std;

// Best-effort extraction of a `"key":"value"` string field from JSON text.
// This project doesn't pull in a full JSON library — the JWT payloads in
// the Login packet only need a handful of known string fields out of them
// (XUID, displayName, identity), so this scans for the key and reads the
// quoted value that follows it directly, instead of parsing the whole
// document into a tree. Pass a substring (e.g. just the "extraData" object)
// to scope the search if the same key could appear elsewhere in the JSON.
string jsonExtractString(const string& json, const string& key);
