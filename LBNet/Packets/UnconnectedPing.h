#pragma once

#include <vector>
#include <arpa/inet.h>

// Decode Unconnected Ping (0x01)
void decodeUnconnectedPing(
    int sock,
    sockaddr_in clientAddr,
    const std::vector<unsigned char>& data
);