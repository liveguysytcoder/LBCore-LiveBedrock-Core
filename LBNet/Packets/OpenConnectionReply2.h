#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include "RaknetGlobals.h"

using namespace std;

// Send Open Connection Reply 2 (0x08)
void sendOpenConnectionReplyTwo(int sock, sockaddr_in clientAddr, unsigned short mtu);