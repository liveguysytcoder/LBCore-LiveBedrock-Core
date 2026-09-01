#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends an EMPTY AvailableCommands (76 / 0x4C) — the full command/chat-
// suggestion registry. One of the deepest packets in the protocol, but a
// real capture confirms every one of its 9 top-level arrays comes back
// zero-length when a server (Dragonfly, here) registers no commands:
// values_len, enum_values, chained_subcommand_values, suffixes, enums,
// chained_subcommands, command_data, dynamic_enums, enum_constraints —
// each just a VarInt(0) count, in that order, with nothing following
// since there's nothing to iterate. This project defines no commands, so
// this is a faithful (not simplified) empty send, not a guess.
void sendAvailableCommands(int sock, sockaddr_in clientAddr, ClientState& state);
