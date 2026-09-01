#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

extern void sendUnconnectedPong(int sock, sockaddr_in clientAddr, const vector<unsigned char>& pingData);

// Decode Unconnected Ping (0x01)
void decodeUnconnectedPing(int sock, sockaddr_in clientAddr, const vector<unsigned char>& data) {

    //cout << "\n🛰️ [Unconnected Ping Detected]\n";

    unsigned long long timestamp = 0;

    if (data.size() >= 9) {
        for (int i = 1; i <= 8; i++)
            timestamp = (timestamp << 8) | data[i];

        //cout << "Client Alive Time (ms): " << timestamp << endl;
    }

    //cout << "RakNet Magic: ";
    for (int i = 9; i < 25 && i < data.size(); i++)
        printf("%02X ", data[i]);

    cout << endl;

    sendUnconnectedPong(sock, clientAddr, data);
}