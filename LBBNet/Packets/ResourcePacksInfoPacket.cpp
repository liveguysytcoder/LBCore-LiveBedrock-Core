#include "ResourcePacksInfoPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include "ResourcePackStackPacket.h"
#include <iostream>

using namespace std;

void sendResourcePacksInfo(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Gamepacket header (varint) for ResourcePacksInfo = 0x06.
    //
    // SIXTH PASS -- this time against Mojang's own official protocol
    // documentation (the field-level packet reference shared with server
    // partners), not a third-party reverse-engineered guess. It confirms
    // the FOURTH-PASS field set was actually right, and my FIFTH-PASS
    // "fix" (which trimmed it down based on gophertunnel's server-side
    // code, which only happened to show two of these bools being *set*,
    // not the packet's full field list) made things worse. Official field
    // list, in order:
    //   0: ResourcePackRequired      (bool)
    //   1: HasAddonPacks             (bool)
    //   2: HasScripts                (bool)
    //   3: ForceDisableVibrantVisuals(bool)
    //   4: WorldTemplateIdAndVersion (PackIdVersion: UUID + SemVersion string)
    //   5: ResourcePacks             (array<PackInfoData>)
    // UUID here is written as Mojang's docs describe it -- most/least
    // significant 64-bit halves, matching a standard Java-style UUID split
    // -- and SemVersion is just a version string.
    vector<uint8_t> packet;
    writeVarInt(packet, 0x06);

    writeBool(packet, false); // ResourcePackRequired
    writeBool(packet, false); // HasAddonPacks
    writeBool(packet, false); // HasScripts
    writeBool(packet, false); // ForceDisableVibrantVisuals

    // WorldTemplateIdAndVersion: PackIdVersion { UUID, SemVersion }
    writeUInt64(packet, 0); // UUID most significant bits -- no world template
    writeUInt64(packet, 0); // UUID least significant bits
    writeString(packet, ""); // SemVersion.Version

    // FIXED: confirmed directly against Mojang's own official protocol
    // docs (mojang.github.io/bedrock-protocol-docs, protocol 2168 == this
    // project's target, 1.26.44). Every dynamic collection length in the
    // protocol uses the "Compression" variable-length encoding (varuint32)
    // UNLESS the field is explicitly annotated "No size compression" --
    // and this field (ResourcePacksInfoPacket.ResourcePacks) carries no
    // such annotation. The earlier "revert to uint16" was based on a
    // local packet-logger capture failing at a reader named "li16", which
    // was a mismatch between that specific test tool's protodef schema
    // and this protocol version -- not evidence about the real wire
    // format. Writing 2 raw bytes here instead of 1 varint byte leaves a
    // stray trailing 0x00 inside this packet's own length-prefixed
    // region, which is exactly the kind of mismatch a strict client can
    // reject as a violation.
    writeVarInt(packet, 0); // TexturePacks/ResourcePacks count -- none served

    sendGamePacket(sock, clientAddr, state, packet);
    cout << "[Bedrock] Sent ResourcePacksInfo\n";

    // REMOVED the eager, unconditional sendResourcePacksStack() call that
    // used to be here. Confirmed directly against gophertunnel's actual
    // CLIENT-side handling (handleResourcePacksInfo / handleResourcePackStack
    // in conn.go): a real client, upon processing ResourcePacksInfo with
    // zero packs to download, sends ResourcePackClientResponse{Response:
    // PackResponseAllPacksDownloaded} ("HaveAllPacks") and then WAITS for
    // ResourcePackStack -- it does not send "Completed" until it actually
    // receives ResourcePackStack afterward. Sending both packets together,
    // unprompted, made a real client receive Info and Stack in the same
    // batch and generate BOTH of its own responses ("HaveAllPacks" and
    // "Completed") in a burst instead of one at a time -- directly
    // observed in this project's own logs as a stray second
    // ResourcePackClientResponse(HaveAllPacks) arriving after StartGame
    // had already been triggered by an early "Completed". The wiki-based
    // comment that used to justify this shortcut doesn't hold up against
    // actual client behavior. ResourcePacksStack is now sent ONLY from
    // handleResourcePackClientResponse's HaveAllPacks case
    // (ResourcePackClientResponsePacket.cpp), in response to the client's
    // own genuine reply -- matching the real Info -> wait -> Stack -> wait
    // -> Completed -> StartGame sequence exactly.
}
