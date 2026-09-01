#include "ServerManager.h"
#include "../../Config/Config.h"
#include <iostream>

using namespace std;

std::string getServerProperties() {
    const ServerConfig& config = getConfig();

    // Bedrock's unconnected-pong MOTD string. Field order is fixed by the
    // protocol: edition;motd;protocol;version;playerCount;maxPlayers;
    // serverGuid;levelName;gamemode;gamemodeNumeric;ipv4Port;ipv6Port;
    return "MCPE;" + config.motd + ";" +
           to_string(config.protocolVersion) + ";" +
           config.versionName + ";" +
           "0;" +
           to_string(config.maxPlayers) + ";" +
           "1122334455667788;" +
           config.levelName + ";" +
           config.gamemode + ";" +
           "1;" +
           to_string(config.port) + ";" +
           to_string(config.port + 1) + ";";
}

int LBStart() {
    cout << "[ServerManager] Starting server...\n";

    start();  // This must be defined in another .cpp file (like Server.cpp)

    cout << "[ServerManager] Server stopped.\n";
    return 0;
}