#include "JsonLite.h"

string jsonExtractString(const string& json, const string& key) {
    string needle = "\"" + key + "\"";
    size_t keyPos = json.find(needle);
    if (keyPos == string::npos) return "";

    size_t colonPos = json.find(':', keyPos + needle.size());
    if (colonPos == string::npos) return "";

    size_t quoteStart = json.find('"', colonPos + 1);
    if (quoteStart == string::npos) return "";

    string value;
    size_t i = quoteStart + 1;
    while (i < json.size() && json[i] != '"') {
        if (json[i] == '\\' && i + 1 < json.size()) {
            // Skip the escape and keep the escaped character literally;
            // good enough for the plain strings (names, UUIDs, numeric
            // XUIDs) these payloads actually carry.
            value += json[i + 1];
            i += 2;
        } else {
            value += json[i];
            i++;
        }
    }
    return value;
}
