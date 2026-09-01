#include "Clients.h"
#include "../../LBBNet/Packets/Encryption.h"
#include <iostream>

using namespace std;

// Assigns numeric client indices in the order clients first connect.
uint16_t nextClientIndex = 0;

// Full per-connection RakNet state, keyed by "ip:port".
map<string, ClientState> clientStates;

string getClientKey(sockaddr_in clientAddr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
    return string(ip) + ":" + to_string(ntohs(clientAddr.sin_port));
}

ClientState& getOrCreateClient(sockaddr_in clientAddr) {
    string key = getClientKey(clientAddr);
    auto it = clientStates.find(key);
    if (it != clientStates.end()) {
        return it->second;
    }

    ClientState state;
    state.index = nextClientIndex++;
    auto result = clientStates.emplace(key, state);
    cout << "[Clients] New client " << key << " assigned index " << state.index << "\n";
    return result.first->second;
}

uint16_t addClient(sockaddr_in clientAddr) {
    return getOrCreateClient(clientAddr).index;
}

void removeClient(sockaddr_in clientAddr) {
    string key = getClientKey(clientAddr);
    auto it = clientStates.find(key);
    if (it != clientStates.end()) {
        destroyClientEncryption(it->second); // frees sendCipherCtx/recvCipherCtx if set
        clientStates.erase(it);
    }
}

bool isClientConnected(sockaddr_in clientAddr) {
    return clientStates.count(getClientKey(clientAddr)) > 0;
}

// Non-creating lookup: returns nullptr if this address has no established
// session, instead of silently creating one. Used for any packet type that
// should only ever apply to an already-connected client (FrameSets, ACKs,
// NACKs) so a stray or retransmitted frame from an address that already
// disconnected doesn't spawn a new phantom client.
ClientState* findExistingClient(sockaddr_in clientAddr) {
    auto it = clientStates.find(getClientKey(clientAddr));
    if (it == clientStates.end()) return nullptr;
    return &it->second;
}
