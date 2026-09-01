#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include "RaknetGlobals.h"

using namespace std;

// Send Open Connection Reply 1 (0x06)
void sendOpenConnectionReplyOne(int sock, sockaddr_in clientAddr, unsigned char protocol);