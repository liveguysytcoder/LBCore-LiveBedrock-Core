#pragma once
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>

using namespace std;

void decodePacket(int sock, sockaddr_in clientAddr, const vector<unsigned char>& data);
void start();
void stopServer();
sockaddr_in getServerAddress();

extern bool running;
extern int serverSocket;
extern sockaddr_in serverAddress;