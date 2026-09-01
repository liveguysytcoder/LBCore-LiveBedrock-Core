#pragma once

#include <vector>
#include <arpa/inet.h>

// Decode Open Connection Request 2 (0x07)
void decodeOpenConnectionRequestTwo(
    int sock,
    sockaddr_in clientAddr,
    const std::vector<unsigned char>& data
);