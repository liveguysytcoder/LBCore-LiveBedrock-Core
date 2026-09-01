#pragma once
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Values for PlayStatus's single status field. Checked against Mojang's
// own field-level packet docs (PlayStatusPacket, id 2) rather than the
// wiki -- names below match what that doc calls each enum value; a couple
// of the mid-range ones were mislabeled here before (e.g. index 6 is
// specifically a vanilla-client-to-Edu-server mismatch, not a generic
// "incompatible pack" failure), and two newer values (8, 9 -- Editor
// mismatches) weren't present here at all.
enum class PlayStatusType : int32_t {
    LoginSuccess                       = 0, // LoginSuccess
    FailedClient                       = 1, // LoginFailed_ClientOld — "Could not connect: Outdated client!"
    FailedServer                       = 2, // LoginFailed_ServerOld — "Could not connect: Outdated server!"
    PlayerSpawn                        = 3, // PlayerSpawn — sent after world data to spawn the player
    FailedInvalidTenant                = 4, // LoginFailed_InvalidTenant — "Your school does not have access to this server."
    FailedEditionMismatchEduToVanilla  = 5, // LoginFailed_EditionMismatchEduToVanilla
    FailedEditionMismatchVanillaToEdu  = 6, // LoginFailed_EditionMismatchVanillaToEdu
    FailedServerFullSubClient          = 7, // LoginFailed_ServerFullSubClient — "Server Full"
    FailedEditorMismatchEditorToVanilla = 8, // LoginFailed_EditorMismatchEditorToVanilla
    FailedEditorMismatchVanillaToEditor = 9, // LoginFailed_EditorMismatchVanillaToEditor
};

// Sends PlayStatus (0x02) to the client. Sending LoginSuccess right after a
// parsed Login packet is what tells the client the server accepted it and
// it's safe to continue the handshake instead of timing out.
void sendPlayStatus(int sock, sockaddr_in clientAddr, ClientState& state, PlayStatusType status);
