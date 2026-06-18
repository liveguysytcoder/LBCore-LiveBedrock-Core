# LBCore-LiveBedrock-Core
LBCore (Live Bedrock Core) is a Minecraft: Bedrock Edition server built from scratch in C++, with its own RakNet-compatible networking layer and Bedrock protocol implementation — no PocketMine, no RakLib, fully independent from the ground up. RakNet handshake, FrameSets, and ACK/NACK work. Currently stuck on one issue: after sending NetworkSettingsPacket (0x8F), the real Bedrock client never sends LoginPacket back — total silence, no response at all. If you've worked with RakNet or Bedrock protocol internals and have ideas what I might be missing, please open an issue. Help is appreciated!
## Known Issue — Login packet not received

I'm currently stuck on a specific bug and could use some outside eyes on it.

After the client sends `RequestNetworkSettingsPacket` (0xC1) and the server replies with `NetworkSettingsPacket` (0x8F), nothing else ever arrives from the real Minecraft: Bedrock Edition client — no `LoginPacket`, no disconnect notification, nothing. The connection just goes silent at that exact point.

If you've worked with RakNet, written a Bedrock server/proxy, or have debugged this kind of handshake before, I'd really appreciate your input. Please open an issue (or comment on the pinned one) describing what you think I might be missing — I'm trying to pin down exactly where the handshake is breaking down.

## Progress so far

- [x] RakNet offline handshake (Unconnected Ping/Pong, Open Connection Request/Reply 1 & 2)
- [x] RakNet connected handshake (Connection Request → Connection Request Accepted → New Incoming Connection)
- [x] Datagram/FrameSet encoding & decoding (reliable, ordered, sequenced frame types)
- [x] ACK/NACK exchange
- [x] RequestNetworkSettingsPacket → NetworkSettingsPacket exchange
- [ ] Compression (zlib)
- [ ] Login packet handling
- [ ] Split-packet reassembly for large packets
- [ ] Resource pack handshake
- [ ] Start game / world join
