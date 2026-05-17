#pragma once
// ─── ESP-NOW mesh discovery: broadcast presence, track peers ────────────────

#include <Arduino.h>
#include <stdint.h>

static constexpr int      MESH_MAX_PEERS    = 8;
static constexpr uint32_t MESH_BEACON_MS    = 5000;
static constexpr uint32_t MESH_PEER_TTL_MS  = 20000;
static constexpr uint8_t  MESH_BEACON_MAGIC = 0xCA;

struct MeshPeer {
    uint8_t  mac[6];
    uint32_t ip;              // network order
    char     name[24];
    uint32_t lastSeen;        // millis()
    bool     active;
};

struct __attribute__((packed)) MeshBeacon {
    uint8_t  magic;           // 0xCA
    uint8_t  version;         // 1
    uint32_t ip;              // sender IP (network byte order)
    char     name[24];        // null-terminated device name
};

void     meshInit();
bool     meshIsReady();
void     meshSendBeacon();
void     meshTick();                     // call periodically from a task
int      meshGetPeers(MeshPeer* out, int maxCount);
int      meshPeerCount();
bool     meshPeersChanged();
String   meshPeersJson();
