#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends the REAL BiomeDefinitionList (122 / 0x7A) — all 87 vanilla biome
// definitions, pulled directly from a real Dragonfly server capture (see
// BiomeDefinitionData.cpp/.h). Sent AFTER PlayStatus(PlayerSpawn) and
// CreativeContent, matching that capture's actual packet order (previous
// revisions of this project sent it right after StartGame, pre-spawn —
// confirmed wrong against the capture; see sendSpawnSequence() in
// ResourcePackClientResponsePacket.cpp).
//
// IMPORTANT: this is NOT the old NBT-compound-of-biome-names format an
// earlier version of this file used. Since roughly the 1.19.70 creative
// menu rework, this packet is a completely different, non-NBT layout: a
// deduplicated string table (string_list) plus an array of definitions
// (biome_definitions) that reference it by index. See the .cpp for the
// exact field order/types.
void sendBiomeDefinitionList(int sock, sockaddr_in clientAddr, ClientState& state);

// Sends a SECOND, genuinely empty BiomeDefinitionList — matching the real
// capture's packet #8 (sent before PlayStatus(PlayerSpawn), right after
// ChunkRadiusUpdated) and gophertunnel's handleRequestChunkRadius()
// (minecraft/conn.go): `_ = conn.WritePacket(&packet.BiomeDefinitionList{})`.
// Present for backwards compatibility with clients older than 1.21.80,
// which crash without it right after ChunkRadiusUpdated (their
// achievement system assumes all biomes are present). Empty in the new
// format means both top-level arrays (biome_definitions, string_list) are
// zero-length — NOT an empty NBT compound, which was this project's
// previous (also wrong) implementation.
void sendEmptyBiomeDefinitionList(int sock, sockaddr_in clientAddr, ClientState& state);
