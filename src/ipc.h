#pragma once
// ─── Inter-core communication: queues + message structs ─────────────────────
//
// All queues carry POINTERS to heap-allocated structs (not copies).
// Convention:  sender malloc()s → enqueues pointer → receiver free()s.
// This avoids copying multi-KB structs through FreeRTOS queue storage.

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>

// ─── Script execution job ───────────────────────────────────────────────────
static constexpr size_t SCRIPT_NAME_MAX = 33;
static constexpr size_t SCRIPT_CODE_MAX = 8192;

struct ScriptJob {
    char     name[SCRIPT_NAME_MAX];
    char     code[SCRIPT_CODE_MAX];
    uint32_t codeLen;
};

// ─── HTTP Fetch IPC ─────────────────────────────────────────────────────────
static constexpr size_t FETCH_URL_MAX      = 256;
static constexpr size_t FETCH_CT_MAX       = 64;
static constexpr size_t FETCH_REQ_BODY_MAX = 2048;
static constexpr size_t FETCH_RSP_BODY_MAX = 4096;

struct FetchRequest {
    uint32_t reqId;
    char     url[FETCH_URL_MAX];
    char     method[8];             // "GET" or "POST"
    char     contentType[FETCH_CT_MAX];
    char     body[FETCH_REQ_BODY_MAX];
    uint32_t bodyLen;
};

struct FetchResponse {
    uint32_t reqId;
    int16_t  httpCode;              // HTTP status, or negative on error
    char     body[FETCH_RSP_BODY_MAX];
    uint32_t bodyLen;
};



// ─── Queue handles (created in main.cpp) ────────────────────────────────────
extern QueueHandle_t g_scriptQueue;      // ScriptJob*     C0/WS/MQTT → C1
extern QueueHandle_t g_fetchReqQueue;    // FetchRequest*  C1 → C0
extern QueueHandle_t g_fetchRspQueue;    // FetchResponse* C0 → C1


// Queue depths
static constexpr int SCRIPT_QUEUE_DEPTH    = 4;
static constexpr int FETCH_QUEUE_DEPTH     = 2;


// Timeout for http.get/post blocking wait (ms)
// Must exceed fetch_worker HTTP timeout (connect 8s + read 12s = 20s)
static constexpr uint32_t FETCH_TIMEOUT_MS = 25000;
