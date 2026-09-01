#pragma once
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../LBNet/Server/Clients.h"

using namespace std;

// Sends UpdateAttributes (25 / 0x19 in the current protocol numbering —
// verified against gophertunnel's packet ID constants: counting
// IDLogin=1 through IDUpdateAttributes gives 25, NOT the 0x2A/42 this
// project's old SetHealth packet used, which was copied from a much older
// protocol doc (1.16.220 / protocol 431) where extra packets like
// TickSync/RiderJump that have since been removed shifted every ID after
// them).
//
// This replaces sendSetHealth(): per the Bedrock protocol docs, SetHealth
// "should no longer be used. Instead, the health attribute should be used
// so that the health and maximum health may be changed directly."
//
// NOTE ON FIELD LAYOUT: gophertunnel's protocol.Attribute struct is
// confirmed (via pkg.go.dev) to contain Name/Value/Max/Min (embedded
// AttributeValue) plus DefaultMin/DefaultMax/Default and a Modifiers
// slice — but the exact *wire order* Marshal() writes those fields in
// couldn't be pulled from source directly (GitHub blocked raw/robots
// access during this session). The layout below follows the order that
// has stayed consistent across every documented Bedrock protocol version
// (Min, Max, Current, [DefaultMin, DefaultMax, Default], Name, Modifiers)
// and matches the struct's field grouping, but — like the VoxelShapes
// packet flagged earlier in this codebase — treat this as needing a
// byte-level diff against a real client capture before relying on it,
// rather than as separately re-verified.
void sendUpdateAttributes(int sock, sockaddr_in clientAddr, ClientState& state,
                           float health, float maxHealth);
