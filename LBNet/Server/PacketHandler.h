#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <arpa/inet.h>

using namespace std;

// Main dispatcher
void decodePacket(int sock, sockaddr_in clientAddr, const vector<unsigned char>& data);
