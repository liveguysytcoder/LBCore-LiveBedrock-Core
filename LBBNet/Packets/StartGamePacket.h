#pragma once
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Enums below are transcribed directly from StartGamePacket's field-level
// protocol doc (packet id 11 / 0x0B). Values matching what's already
// declared elsewhere in this project (GameType, PlayStatusType's spawn/fail
// codes, etc.) are intentionally re-declared here rather than shared, same
// as the rest of LBBNet/Packets does per-packet.

enum class GameType : int32_t {
    Undefined    = 0,
    Survival     = 1,
    Creative     = 2,
    Adventure    = 3,
    Default      = 4,
    Spectator    = 5,
    WorldDefault = 6,
};

enum class SpawnBiomeType : int16_t {
    Default     = 0,
    UserDefined = 1,
};

// LevelSettings.GeneratorType — the world generator, distinct from
// SpawnSettings.Dimension (which the doc leaves as a plain varint32 with
// no enum table of its own; standard Bedrock dimension ids are
// 0 = Overworld, 1 = Nether, 2 = TheEnd).
enum class GeneratorType : int32_t {
    Legacy    = 0,
    Overworld = 1,
    Flat      = 2,
    Nether    = 3,
    TheEnd    = 4,
    Void      = 5,
    Undefined = 6,
};

enum class LevelDifficulty : int32_t {
    Peaceful = 0,
    Easy     = 1,
    Normal   = 2,
    Hard     = 3,
    Count    = 4,
    Unknown  = 5,
};

enum class EditorWorldType : int32_t {
    NonEditor          = 0,
    EditorProject      = 1,
    EditorTestLevel    = 2,
    EditorRealmsUpload = 3,
};

enum class EducationEditionOffer : uint32_t {
    None            = 0,
    RestOfWorld     = 1,
    China_Deprecated = 2,
};

// Shared by both Xbox Live Broadcast Setting and Platform Broadcast Setting
// — the doc lists the exact same enum table for each.
enum class BroadcastSetting : int32_t {
    NoMultiPlay      = 0,
    InviteOnly       = 1,
    FriendsOnly      = 2,
    FriendsOfFriends = 3,
    Public           = 4,
};

enum class PlayerPermissionLevel : int8_t {
    Visitor  = 0,
    Member   = 1,
    Operator = 2,
    Custom   = 3,
};

enum class ChatRestrictionLevel : uint8_t {
    None     = 0,
    Dropped  = 1,
    Disabled = 2,
};

enum class ServerEditorConnectionPolicy : int32_t {
    MatchWorldType = 0,
    EditorOnly     = 1,
    VanillaOnly    = 2,
    Mixed          = 3,
};

// Sends StartGame (11 / 0x0B) to the client — tells it the game is
// starting, with the player's ids, spawn position, and the full world/level
// settings block. Per the VoxelShapesPacket doc ("This packet should
// always be sent before StartGamePacket"), this always sends
// VoxelShapesPacket first.
//
// This project has exactly one player per connection and no real world
// state yet, so the ids/settings sent here are fixed stub values with the
// right shape rather than anything read from a loaded world — same
// approach as the rest of LBBNet/Packets (ResourcePacksInfo, PlayStatus,
// etc.) takes for fields that don't have real data behind them yet.
void sendStartGame(int sock, sockaddr_in clientAddr, ClientState& state);
