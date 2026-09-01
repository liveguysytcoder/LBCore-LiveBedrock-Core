#include <iostream>
#include "LBNet/Server/ServerManager.h"
#include "LBBNet/Server/ServerManager.h"
#include "LBBNet/Packets/Encryption.h"
#include "Logging/Logger.h"
#include "Config/Config.h"

int main() {
    // Must run before anything else prints — it takes over std::cout so
    // every line from here on is also written to lbcore.log.txt (which is
    // truncated first, so it only ever holds this run's output).
    initLogger();

    // Reads server.properties (port, bind-address, motd, etc) if present
    // next to the executable; falls back to built-in defaults otherwise.
    // Must run before LBStart(), since Server.cpp reads the port/bind
    // address from this at bind() time.
    loadConfig();

    // Needed before the first client can log in — real (Xbox Live signed
    // in, online) clients go through the encryption handshake right after
    // Login, which needs this key pair. See LBBNet/Packets/Encryption.h.
    generateServerKeyPair();

    LBBStart();
    LBStart();
    return 0;
}