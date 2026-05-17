// ─── mesh.cpp — ESP-NOW mesh discovery ──────────────────────────────────────
// Broadcasts a beacon every MESH_BEACON_MS with this device's name + IP.
// Receives beacons from other devices and maintains a peer table.
// Peers expire after MESH_PEER_TTL_MS of silence.

#include "mesh.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "config_store.h"

static MeshPeer peers[MESH_MAX_PEERS];
static int peerCount = 0;
static uint32_t lastBeaconMs = 0;
static bool meshReady = false;
static int lastPeerCount = -1;  // track changes for display update

// Broadcast address for ESP-NOW
static const uint8_t BROADCAST_ADDR[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ─── Send callback for debug ────────────────────────────────────────────────
static void onSend(const uint8_t *mac, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        DBG("mesh", "Beacon send failed (status=%d)", status);
    }
}

// ─── Receive callback ───────────────────────────────────────────────────────
static void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len < (int)sizeof(MeshBeacon)) return;
    const MeshBeacon* beacon = (const MeshBeacon*)data;
    if (beacon->magic != MESH_BEACON_MAGIC || beacon->version != 1) return;

    // Ignore our own beacons
    uint32_t myIp = (uint32_t)WiFi.localIP();
    if (beacon->ip == myIp) return;

    // Find existing or free slot
    int freeSlot = -1;
    for (int i = 0; i < MESH_MAX_PEERS; i++) {
        if (peers[i].active && memcmp(peers[i].mac, mac, 6) == 0) {
            // Update existing peer
            peers[i].ip = beacon->ip;
            strncpy(peers[i].name, beacon->name, sizeof(peers[i].name) - 1);
            peers[i].name[sizeof(peers[i].name) - 1] = '\0';
            peers[i].lastSeen = millis();
            return;
        }
        if (!peers[i].active && freeSlot == -1) freeSlot = i;
    }

    if (freeSlot == -1) {
        // Evict oldest
        uint32_t oldest = UINT32_MAX;
        for (int i = 0; i < MESH_MAX_PEERS; i++) {
            if (peers[i].lastSeen < oldest) { oldest = peers[i].lastSeen; freeSlot = i; }
        }
    }

    if (freeSlot >= 0) {
        memcpy(peers[freeSlot].mac, mac, 6);
        peers[freeSlot].ip = beacon->ip;
        strncpy(peers[freeSlot].name, beacon->name, sizeof(peers[freeSlot].name) - 1);
        peers[freeSlot].name[sizeof(peers[freeSlot].name) - 1] = '\0';
        peers[freeSlot].lastSeen = millis();
        peers[freeSlot].active = true;

        // Recount
        peerCount = 0;
        for (int i = 0; i < MESH_MAX_PEERS; i++) {
            if (peers[i].active) peerCount++;
        }

        IPAddress ip(peers[freeSlot].ip);
        DBG("mesh", "New peer: %s @ %s", peers[freeSlot].name, ip.toString().c_str());
    }
}

// ─── Init ───────────────────────────────────────────────────────────────────
void meshInit() {
    memset(peers, 0, sizeof(peers));
    peerCount = 0;
    lastBeaconMs = 0;
    meshReady = false;
    lastPeerCount = 0;

    if (WiFi.status() != WL_CONNECTED) {
        DBG("mesh", "WiFi not connected — skipping mesh init");
        return;
    }

    if (esp_now_init() != ESP_OK) {
        DBG("mesh", "ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(onRecv);
    esp_now_register_send_cb(onSend);

    // Get current WiFi channel — ESP-NOW must use the same channel
    uint8_t primaryChan = 0;
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&primaryChan, &secondChan);
    DBG("mesh", "WiFi channel: %d", primaryChan);

    // Add broadcast peer on the WiFi channel
    esp_now_peer_info_t bcastPeer = {};
    memcpy(bcastPeer.peer_addr, BROADCAST_ADDR, 6);
    bcastPeer.channel = primaryChan;  // must match WiFi STA channel
    bcastPeer.encrypt = false;
    bcastPeer.ifidx = WIFI_IF_STA;

    if (!esp_now_is_peer_exist(BROADCAST_ADDR)) {
        esp_err_t err = esp_now_add_peer(&bcastPeer);
        if (err != ESP_OK) {
            DBG("mesh", "Failed to add broadcast peer: %d", err);
        }
    }

    meshReady = true;
    lastPeerCount = 0;
    DBG("mesh", "ESP-NOW mesh initialized on Core %d, chan %d", xPortGetCoreID(), primaryChan);
}

bool meshIsReady() {
    return meshReady;
}

// ─── Send beacon ────────────────────────────────────────────────────────────
void meshSendBeacon() {
    if (!meshReady) return;

    MeshBeacon beacon;
    beacon.magic = MESH_BEACON_MAGIC;
    beacon.version = 1;
    beacon.ip = (uint32_t)WiFi.localIP();
    memset(beacon.name, 0, sizeof(beacon.name));
    KairosConfig& cfg = configGet();
    strncpy(beacon.name, cfg.device_name.c_str(), sizeof(beacon.name) - 1);

    esp_now_send(BROADCAST_ADDR, (uint8_t*)&beacon, sizeof(beacon));
}

// ─── Tick — send beacons + expire stale peers ───────────────────────────────
void meshTick() {
    if (!meshReady) return;

    uint32_t now = millis();

    // Send beacon periodically
    if (now - lastBeaconMs >= MESH_BEACON_MS) {
        meshSendBeacon();
        lastBeaconMs = now;
    }

    // Expire stale peers
    for (int i = 0; i < MESH_MAX_PEERS; i++) {
        if (peers[i].active && (now - peers[i].lastSeen) > MESH_PEER_TTL_MS) {
            DBG("mesh", "Peer expired: %s", peers[i].name);
            peers[i].active = false;
            peerCount = 0;
            for (int j = 0; j < MESH_MAX_PEERS; j++) {
                if (peers[j].active) peerCount++;
            }
        }
    }
}

// ─── Query peers ────────────────────────────────────────────────────────────
int meshGetPeers(MeshPeer* out, int maxCount) {
    int n = 0;
    for (int i = 0; i < MESH_MAX_PEERS && n < maxCount; i++) {
        if (peers[i].active) {
            out[n++] = peers[i];
        }
    }
    return n;
}

int meshPeerCount() {
    return peerCount;
}

bool meshPeersChanged() {
    if (!meshReady) return false;
    if (peerCount != lastPeerCount) {
        lastPeerCount = peerCount;
        return true;
    }
    return false;
}

String meshPeersJson() {
    JsonDocument doc;

    // Include self as first entry
    JsonObject self = doc.add<JsonObject>();
    self["name"] = configGet().device_name;
    self["ip"] = WiFi.localIP().toString();
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    self["mac"] = macStr;
    self["self"] = true;

    // Add peers
    for (int i = 0; i < MESH_MAX_PEERS; i++) {
        if (!peers[i].active) continue;
        JsonObject p = doc.add<JsonObject>();
        p["name"] = peers[i].name;
        IPAddress ip(peers[i].ip);
        p["ip"] = ip.toString();
        char pMac[18];
        snprintf(pMac, sizeof(pMac), "%02X:%02X:%02X:%02X:%02X:%02X",
                 peers[i].mac[0], peers[i].mac[1], peers[i].mac[2],
                 peers[i].mac[3], peers[i].mac[4], peers[i].mac[5]);
        p["mac"] = pMac;
        p["self"] = false;
        p["age_ms"] = millis() - peers[i].lastSeen;
    }

    String out;
    serializeJson(doc, out);
    return out;
}
