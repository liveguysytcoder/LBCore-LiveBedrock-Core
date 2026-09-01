#include "UpdateAttributesPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

namespace {

// Writes a single Attribute (AttributeData) entry. Field order confirmed
// directly against Mojang's own official protocol docs for protocol 2168
// (this project's target, 1.26.44): Min, Max, Current, DefaultMin,
// DefaultMax, Default, Name, Modifiers -- in that exact order. The
// previous version wrote Name FIRST and Current/Max/Min out of order,
// which is what actually produced the "found size 45 expected size was
// 93" string-length error downstream: a reader expecting a Min float here
// was instead handed the first bytes of the length-prefixed Name string,
// and vice versa, so every field after the swap landed on the wrong byte
// offsets. Name's wire type is `hashed_string`, but per the docs site
// that type carries no extra on-wire fields beyond the usual
// varuint32-length-prefixed string -- the "hashed" part is a client-side
// runtime cache key derived FROM the string, not a separate byte on the
// wire -- so writeString() here is already correct.
void writeAttribute(vector<uint8_t>& packet, const string& name,
                     float min, float max, float current,
                     float defaultMin, float defaultMax, float defaultValue) {
    writeFloat(packet, min);
    writeFloat(packet, max);
    writeFloat(packet, current);

    writeFloat(packet, defaultMin);
    writeFloat(packet, defaultMax);
    writeFloat(packet, defaultValue);

    writeString(packet, name); // Name (hashed_string -- same wire format as string)

    writeVarInt(packet, 0); // Modifiers: array count = 0
}

} // namespace

void sendUpdateAttributes(int sock, sockaddr_in clientAddr, ClientState& state,
                           float health, float maxHealth) {
    // Gamepacket header (varint) for UpdateAttributes.
    //
    // FIXED: this was 25 (0x19) — confirmed WRONG. Cross-checking
    // PrismarineJS/minecraft-data's current bedrock proto.yml (the same
    // schema that decoded this project's own real-client capture) shows
    // 0x19 is explicitly `packet_level_event`, not update_attributes. That
    // exactly explains a real symptom seen in testing: a phantom
    // "level_event" packet with garbage/insane position values appearing
    // right where UpdateAttributes gets sent — the client was decoding
    // these bytes using LevelEvent's schema instead. The earlier "count
    // packet IDs sequentially from IDLogin=1" approach used to arrive at
    // 25 doesn't hold up: real packet IDs have gaps from removed packets
    // (e.g. a slot at 0x10 and RiderJump at 0x14, both still reserved even
    // though unused/removed) that a plain sequential count skips over.
    // Counting gophertunnel's actual registration order against the
    // schema's confirmed numeric IDs instead
    // (0x19 level_event, 0x1a block_event, 0x1b entity_event,
    // 0x1c mob_effect, 0x1d update_attributes) puts UpdateAttributes at
    // 29 (0x1D).
    vector<uint8_t> packet;
    writeVarInt(packet, 29);

    // Runtime Entity ID (varuint64) — this project has exactly one player
    // per connection, fixed at 1 (see StartGamePacket.cpp).
    writeVarInt64(packet, 1);

    // Attributes: varint-prefixed array. Only minecraft:health is sent —
    // this is a direct, minimal replacement for the old SetHealth call.
    // Add further entries here (movement, hunger, etc.) the same way if
    // this ever needs to cover more than health.
    writeVarInt(packet, 1); // Attributes: array count = 1
    writeAttribute(packet, "minecraft:health",
                    /*min*/ 0.0f, /*max*/ maxHealth, /*current*/ health,
                    /*defaultMin*/ 0.0f, /*defaultMax*/ maxHealth, /*defaultValue*/ maxHealth);

    // FIXED: this packet has a 3rd top-level field per the official docs --
    // Tick (PlayerInputTick, varuint64) -- that was missing entirely. A
    // reader that trusts the schema will always try to read it, so leaving
    // it off shortens the packet by (at least) one byte versus what the
    // client expects. 0 is a safe placeholder; wire in the real current
    // server tick here once this project is tracking one.
    writeVarInt64(packet, 0); // Tick

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent UpdateAttributes(health=" << health << "/" << maxHealth << ")\n";
}
