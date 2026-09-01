cmake_minimum_required(VERSION 3.10)
project(Lbcore LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Update Include Directories
# Added subfolders so executer.cpp can find your .h files easily
include_directories(
    .
    LBNet
    LBNet/Packets
    LBNet/Server
    LBBNet
    LBBNet/Packets
    LBBNet/Server
    Logging
    Config
)

# 2. Collect source files
file(GLOB_RECURSE LBNET_SOURCES "LBNet/*.cpp")
file(GLOB_RECURSE LBBNET_SOURCES "LBBNet/*.cpp")
file(GLOB_RECURSE LOGGING_SOURCES "Logging/*.cpp")
file(GLOB_RECURSE CONFIG_SOURCES "Config/*.cpp")

# 3. Create the final source list including the new executer.cpp
set(ALL_SOURCES 
    executer.cpp 
    ${LBNET_SOURCES} 
    ${LBBNET_SOURCES}
    ${LOGGING_SOURCES}
    ${CONFIG_SOURCES}
)

# 4. Create the single executable
add_executable(LBCore ${ALL_SOURCES})

# 5. Link zlib (used by LBBNet/Packets/Compression.cpp)
find_package(ZLIB REQUIRED)
target_link_libraries(LBCore PRIVATE ZLIB::ZLIB)

# 6. Link OpenSSL's crypto lib (EC keygen/ECDH/ECDSA/AES — used by
#    LBBNet/Packets/Encryption.cpp for the login encryption handshake)
find_package(OpenSSL REQUIRED)
target_link_libraries(LBCore PRIVATE OpenSSL::Crypto)
