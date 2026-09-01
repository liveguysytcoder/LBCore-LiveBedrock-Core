#include "ResourcePackClientResponsePacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include "ResourcePackStackPacket.h"
#include "StartGamePacket.h"
#include "LevelChunkPacket.h"
#include "NetworkChunkPublisherUpdatePacket.h"
#include "PlayStatusPacket.h"
#include "InventoryContentPacket.h"
#include "AvailableActorIdentifiersPacket.h"
#include "BiomeDefinitionListPacket.h"
#include "CreativeContentPacket.h"
#include "CraftingDataPacket.h"
#include "SetSpawnPositionPacket.h"
#include "UpdateAttributesPacket.h"
#include "TrimDataPacket.h"
#include "SetTimePacket.h"
#include "GameRulesChangedPacket.h"
#include "LevelEventPacket.h"
#include "SetPlayerGameTypePacket.h"
#include "UpdatePlayerGameTypePacket.h"
#include "UpdateAbilitiesPacket.h"
#include "SetEntityDataPacket.h"
#include "TextPacket.h"
#include "AvailableCommandsPacket.h"
#include <iostream>

using namespace std;

void handleResourcePackClientResponse(int sock, sockaddr_in clientAddr, ClientState& state,
                                       const vector<uint8_t>& data, size_t offset) {
    if (offset >= data.size()) {
        cout << "[ResourcePackClientResponse] Packet too short for status, dropping\n";
        return;
    }
    auto status = static_cast<ResourcePackResponseStatus>(readUByte(data, offset));

    // Only present when status == SendPacks: a list of pack ids the client
    // wants downloaded. We never offer any packs, so this should never
    // actually happen — but a real client could still send an empty list
    // here, so read (and discard) it defensively to keep offset sane for
    // anyone parsing further, rather than assuming the field is absent.
    if (status == ResourcePackResponseStatus::SendPacks && offset + 2 <= data.size()) {
        uint16_t packCount = readUShort(data, offset);
        for (uint16_t i = 0; i < packCount && offset < data.size(); i++) {
            readString(data, offset);
        }
    }

    switch (status) {
        case ResourcePackResponseStatus::Refused:
            cout << "[ResourcePackClientResponse] Refused — disconnecting isn't implemented "
                    "yet, but the client has declined to continue\n";
            break;

        case ResourcePackResponseStatus::SendPacks:
            // We have no packs to send; nothing to do but wait for the
            // client to re-check and report HaveAllPacks.
            cout << "[ResourcePackClientResponse] SendPacks requested, but no packs are served\n";
            break;

        case ResourcePackResponseStatus::HaveAllPacks:
            cout << "[ResourcePackClientResponse] HaveAllPacks\n";
            // Guard against resending ResourcePacksStack if the resource
            // pack stage (and everything after it — StartGame,
            // ItemRegistry, ...) has already completed. A real client can
            // apparently send a second HaveAllPacks after already
            // receiving StartGame+ItemRegistry (confirmed: this isn't a
            // frame-ordering artifact — the Reliable-Ordered delivery fix
            // in PacketHandler.cpp still shows this as a genuine second,
            // later message, not a reordered one). Resending
            // ResourcePacksStack at that point — telling an already-mid-
            // spawn client to go back to resource-pack negotiation — is
            // protocol-nonsensical and could plausibly be what confuses a
            // real client into giving up. Once startGameSent is true,
            // there's nothing useful to do with a stray HaveAllPacks;
            // just acknowledge it in the log and otherwise ignore it.
            if (state.startGameSent) {
                cout << "[ResourcePackClientResponse] Stage already complete "
                        "(StartGame already sent) — ignoring stray HaveAllPacks, "
                        "NOT resending ResourcePacksStack\n";
                break;
            }
            sendResourcePacksStack(sock, clientAddr, state);
            break;

        case ResourcePackResponseStatus::Completed:
            // Resource pack stage is done per a real client response.
            cout << "[ResourcePackClientResponse] Completed — resource pack stage done\n";
            completeResourcePackStageAndStartGame(sock, clientAddr, state);
            break;
    }
}

void completeResourcePackStageAndStartGame(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Guard against sending this twice — see the comment on
    // ClientState::startGameSent in Clients.h for why that's possible now.
    if (state.startGameSent) {
        cout << "[StartGame] Already sent for this client, skipping duplicate trigger\n";
        return;
    }
    state.startGameSent = true;

    // StartGame (+ ItemRegistry, sent from inside sendStartGame itself) is
    // ALL that happens here. A real Dragonfly capture (packet-logger_log.txt)
    // was checked directly, packet-by-packet, and confirmed that every
    // other definition/world/inventory packet this project sends
    // (AvailableActorIdentifiers, the real BiomeDefinitionList, CraftingData,
    // SetSpawnPosition, UpdateAttributes, chunks, NetworkChunkPublisherUpdate,
    // InventoryContent, ...) is actually sent AFTER PlayStatus(PlayerSpawn)
    // in the real sequence, not before it — an earlier pass of this file
    // had all of that here, which was wrong. See sendSpawnSequence() below
    // for the corrected, capture-verified order.
    cout << "[StartGame] Sending StartGame\n";
    sendStartGame(sock, clientAddr, state);
}

void sendSpawnSequence(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Full post-RequestChunkRadius sequence, reconstructed by reading a
    // real Dragonfly server capture (packet-logger_log.txt) packet by
    // packet, not by guessing from gophertunnel's minimal built-in
    // handler (which only covers 4 of these — gophertunnel is a low-level
    // library; Dragonfly is a full game server built on top of it that
    // sends a lot more from its own application layer). Numbers below are
    // that capture's packet indices, for anyone diffing against it again:
    //   #7  ChunkRadiusUpdated        -- sent by the caller in PacketHandler.cpp
    //   #8  BiomeDefinitionList EMPTY
    //   #9  PlayStatus(PlayerSpawn)
    //   === SPAWNED ===
    //   #10 CreativeContent EMPTY
    //   #12 BiomeDefinitionList REAL (87 entries — #11 was a duplicate
    //        empty ItemRegistry, skipped, see note at the end of this
    //        function for everything intentionally left out)
    //   #14 CraftingData
    //   #15 TrimData
    //   #16 UpdateAttributes
    //   #21 SetTime
    //   #22 GameRulesChanged
    //   #23-24 LevelEvent (StopRain, StopThunder)
    //   #25 SetSpawnPosition
    //   #26 NetworkChunkPublisherUpdate
    //   #27 AvailableActorIdentifiers
    //   #28 SetPlayerGameType
    //   #29 UpdateAbilities
    //   #30 UpdatePlayerGameType
    //   #31 SetEntityData
    //   #32-35 InventoryContent (this project sends 3 windows, not 4 — see note)
    //   #36 Text (join message)
    //   #37 AvailableCommands EMPTY
    //   #39 LevelChunk (chunk streaming starts)
    if (state.spawnSequenceSent) {
        cout << "[Spawn] Already sent for this client, skipping duplicate trigger\n";
        return;
    }
    state.spawnSequenceSent = true;

    sendEmptyBiomeDefinitionList(sock, clientAddr, state);
    sendPlayStatus(sock, clientAddr, state, PlayStatusType::PlayerSpawn);
    sendCreativeContent(sock, clientAddr, state);

    sendBiomeDefinitionList(sock, clientAddr, state); // the REAL 87-entry one now
    sendCraftingData(sock, clientAddr, state);
    sendEmptyTrimData(sock, clientAddr, state);
    sendUpdateAttributes(sock, clientAddr, state, 20.0f, 20.0f);
    sendSetTime(sock, clientAddr, state, 1);
    sendGameRulesChanged(sock, clientAddr, state, "dodaylightcycle", /*editable=*/false, /*value=*/true);
    sendLevelEvent(sock, clientAddr, state, LevelEventType::StopRain, 0, 0, 0, 0);
    sendLevelEvent(sock, clientAddr, state, LevelEventType::StopThunder, 0, 0, 0, 0);

    // StartGame's spawn position is (0, 4, 0) — chunk (0, 0).
    sendSetSpawnPosition(sock, clientAddr, state, SpawnPositionType::WorldSpawn, 0, 4, 0, 0);
    // FIXED: this was 64 (blocks) = a 4-chunk radius, but the chunk-send
    // loop below only ever sends a 3x3 grid (1-chunk radius = 16 blocks)
    // around spawn. NetworkChunkPublisherUpdate is the server telling the
    // client "everything within this radius is available now" -- claiming
    // 64 while only actually publishing 16 worth of terrain meant a real
    // client would wait for chunks it was promised but that never arrive,
    // hanging on the loading screen until it gives up and disconnects.
    // NOTE: this alone turned out not to be enough -- ChunkRadiusUpdated
    // (sent much earlier, in PacketHandler.cpp) was STILL echoing the
    // client's full requested radius (e.g. 8) instead of being clamped to
    // this same number, so the client was told two different, contradictory
    // things about its view distance. Both are now driven off
    // kSentChunkRadius (see LevelChunkPacket.h) so they can't disagree
    // again. If the chunk-send loop below is ever widened to cover more
    // area, bump kSentChunkRadius and this stays in sync automatically.
    sendNetworkChunkPublisherUpdate(sock, clientAddr, state, 0, 4, 0, kSentChunkRadius * 16);
    sendAvailableActorIdentifiers(sock, clientAddr, state);

    sendSetPlayerGameType(sock, clientAddr, state, 1); // creative, matches StartGame's player_gamemode
    sendUpdatePlayerGameType(sock, clientAddr, state, /*gamemode=*/1, /*playerUniqueId=*/1, /*tick=*/0);

    // FIXED: these two were fully written (see their own .cpp/.h files
    // and header comments for what's confirmed vs. best-effort about each)
    // but never actually called from here -- the real spawn burst was
    // silently missing both. UpdateAbilities in particular is a very
    // commonly-reported cause of a real client hanging on the loading
    // screen forever in homebrew Bedrock server implementations: without
    // it, the client has no idea what the player is allowed to do and
    // apparently won't finish considering itself "spawned". Matches the
    // real capture's order too (#29 UpdateAbilities, #31 SetEntityData,
    // both between UpdatePlayerGameType/#30 and InventoryContent/#32).
    sendUpdateAbilities(sock, clientAddr, state);
    sendSetEntityData(sock, clientAddr, state);

    // The client builds its inventory/hotbar HUD right as it spawns in.
    sendInventoryContent(sock, clientAddr, state, InventoryWindowId::Inventory);
    sendInventoryContent(sock, clientAddr, state, InventoryWindowId::Armor);
    sendInventoryContent(sock, clientAddr, state, InventoryWindowId::OffHand);

    sendJoinMessage(sock, clientAddr, state, state.displayName);
    sendAvailableCommands(sock, clientAddr, state);

    // Chunk streaming itself starts here too in the real capture (#39),
    // not pre-spawn — send the spawn chunk plus its immediate neighbors
    // (real superflat terrain, see LevelChunkPacket.h) now. Bounded by
    // kSentChunkRadius, same constant ChunkRadiusUpdated and
    // NetworkChunkPublisherUpdate were clamped to above -- widen that one
    // constant to send more than a 3x3 patch, don't just change the loop
    // bounds here in isolation (that's what caused the mismatch bug).
    for (int32_t cx = -kSentChunkRadius; cx <= kSentChunkRadius; cx++) {
        for (int32_t cz = -kSentChunkRadius; cz <= kSentChunkRadius; cz++) {
            sendLevelChunk(sock, clientAddr, state, cx, cz);
        }
    }

    // Deliberately NOT sent — each was present in the real capture but
    // skipped here, either because it's a low-value exact duplicate of
    // something already sent, or because its wire format carries enough
    // risk/complexity that sending it wrong seemed worse than not sending
    // it at all (none of these look load-bearing for spawning/rendering):
    //   - A second, empty ItemRegistry (#11) and a second, REAL,
    //     populated CreativeContent (#13) — sending a populated
    //     CreativeContent that references real item network IDs would be
    //     actively inconsistent with this project's own ItemRegistry,
    //     which defines zero items; better to stay consistent and empty.
    //   - UpdateAbilities (#29) and SetEntityData (#31) — NO LONGER
    //     skipped, now sent (see UpdateAbilitiesPacket.cpp /
    //     SetEntityDataPacket.cpp for what's confirmed vs. best-effort
    //     about each).
    //   - PlayerList (#17) — carries full skin/geometry data per entry in
    //     the real capture (5MB+ for one player); tab-list UI only, not
    //     needed to render the world.
    //   - 3 duplicate UpdateAttributes (#18-20), a duplicate
    //     NetworkChunkPublisherUpdate (#38), and a 4th InventoryContent
    //     window (#32-35 shows 4, this sends 3) — all appear to be
    //     inconsequential repeats/extras in the capture.

    flushGamePackets(sock, clientAddr, state);
}
