#pragma once

#include <vector>
#include <arpa/inet.h>

// Decode Open Connection Request 1 (0x05)
void decodeOpenConnectionRequestOne(
    int sock,
    sockaddr_in clientAddr,
    const std::vector<unsigned char>& data
);