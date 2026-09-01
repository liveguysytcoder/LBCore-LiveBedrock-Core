#include "CraftingDataPacket.h"
#include "DataTypeHelper.h"
#include "GamePacketSender.h"
#include <iostream>

using namespace std;

void sendCraftingData(int sock, sockaddr_in clientAddr, ClientState& state) {
    // Gamepacket header (varint) for CraftingData = 52 (0x34).
    //
    // FIXED AGAIN: confirmed directly against Mojang's official protocol
    // docs (protocol 2168 / 1.26.44) rather than a third-party schema.
    // The real packet is NOT "one array of tagged recipes + 3 more
    // arrays + a bool" (4 arrays + 1 bool, what the previous fix here
    // assumed) -- it's 11 SEPARATE, individually-typed arrays followed by
    // the bool (12 top-level fields total):
    //   0  Shaped Recipes                 array<ShapedRecipePayload>
    //   1  Shapeless Recipes              array<ShapelessRecipePayload>
    //   2  Multi Recipes                  array<MultiRecipePayload>
    //   3  User Data Shapeless Recipes    array<ShapelessRecipePayload>
    //   4  Shapeless Chemistry Recipes    array<ShapelessRecipePayload>
    //   5  Shaped Chemistry Recipes       array<ShapedRecipePayload>
    //   6  Smithing Transform Recipes     array<SmithingTransformRecipePayload>
    //   7  Smithing Trim Recipes          array<SmithingTrimRecipePayload>
    //   8  Potion Mixes                   array<PotionMixDataEntry>
    //   9  Container Mixes                array<ContainerMixDataEntry>
    //   10 Material Reducers              array<MaterialReducerDataEntry>
    //   11 Clear Recipes                  boolean
    // Sending only 4 counts left the reader expecting 7 more varints that
    // never came, which is exactly the "Unexpected buffer end while
    // reading VarInt" seen in testing -- it wasn't a poisoned read from a
    // neighboring packet (each packet in this project's batches is its
    // own accurately length-prefixed region, confirmed via
    // GamePacketSender.cpp), it was this packet legitimately running out
    // of its own bytes.
    vector<uint8_t> packet;
    writeVarInt(packet, 52);

    writeVarInt(packet, 0); // Shaped Recipes: array count = 0
    writeVarInt(packet, 0); // Shapeless Recipes: array count = 0
    writeVarInt(packet, 0); // Multi Recipes: array count = 0
    writeVarInt(packet, 0); // User Data Shapeless Recipes: array count = 0
    writeVarInt(packet, 0); // Shapeless Chemistry Recipes: array count = 0
    writeVarInt(packet, 0); // Shaped Chemistry Recipes: array count = 0
    writeVarInt(packet, 0); // Smithing Transform Recipes: array count = 0
    writeVarInt(packet, 0); // Smithing Trim Recipes: array count = 0
    writeVarInt(packet, 0); // Potion Mixes: array count = 0
    writeVarInt(packet, 0); // Container Mixes: array count = 0
    writeVarInt(packet, 0); // Material Reducers: array count = 0

    writeBool(packet, false); // Clear Recipes — keep the client's own defaults

    queueGamePacket(state, packet);
    cout << "[Bedrock] Sent CraftingData\n";
}
