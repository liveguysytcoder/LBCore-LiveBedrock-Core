#pragma once
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "RaknetGlobals.h"

using namespace std;

void sendUnconnectedPong(int sock, sockaddr_in clientAddr, const vector<unsigned char>& pingData);
