// ─── fetch_worker.cpp — Core 0 task: dequeue HTTP requests, execute, respond ─
// Elk's http.get()/http.post() on Core 1 push FetchRequest* to g_fetchReqQueue,
// then block on g_fetchRspQueue.  This task performs the actual HTTP call on
// Core 0 (where WiFi/LWIP lives) and pushes FetchResponse* back.

#include "fetch_worker.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "config.h"
#include "ipc.h"

static volatile bool taskShouldRun = false;

void fetchWorkerTask(void* param) {
    (void)param;
    taskShouldRun = true;
    DBG("fetch", "Fetch worker running on Core %d", xPortGetCoreID());

    for (;;) {
        if (!taskShouldRun) break;

        FetchRequest* req = nullptr;
        if (xQueueReceive(g_fetchReqQueue, &req, pdMS_TO_TICKS(500)) != pdTRUE)
            continue;
        if (!req) continue;

        DBG("fetch", "%s %s", req->method, req->url);

        FetchResponse* rsp = (FetchResponse*)malloc(sizeof(FetchResponse));
        if (!rsp) {
            DBG("fetch", "Failed to alloc response");
            free(req);
            continue;
        }
        memset(rsp, 0, sizeof(FetchResponse));
        rsp->reqId = req->reqId;

        HTTPClient http;
        http.setTimeout(12000);
        http.setConnectTimeout(8000);

        bool isHttps = (strncmp(req->url, "https", 5) == 0);

        // WiFiClientSecure must outlive HTTPClient usage — declare here
        WiFiClientSecure secureClient;
        WiFiClient       plainClient;

        bool begun = false;
        if (isHttps) {
            secureClient.setInsecure();   // skip cert validation
            begun = http.begin(secureClient, req->url);
        } else {
            begun = http.begin(plainClient, req->url);
        }

        if (!begun) {
            rsp->httpCode = -1;
            strncpy(rsp->body, "connect failed", FETCH_RSP_BODY_MAX - 1);
            rsp->bodyLen = strlen(rsp->body);
        } else {
            int code;
            if (strcmp(req->method, "POST") == 0) {
                if (req->contentType[0])
                    http.addHeader("Content-Type", req->contentType);
                code = http.POST((uint8_t*)req->body, req->bodyLen);
            } else {
                code = http.GET();
            }

            rsp->httpCode = (int16_t)code;
            if (code > 0) {
                String payload = http.getString();
                size_t len = payload.length();
                if (len >= FETCH_RSP_BODY_MAX) len = FETCH_RSP_BODY_MAX - 1;
                memcpy(rsp->body, payload.c_str(), len);
                rsp->body[len] = '\0';
                rsp->bodyLen = len;
            } else {
                snprintf(rsp->body, FETCH_RSP_BODY_MAX, "HTTP error %d", code);
                rsp->bodyLen = strlen(rsp->body);
            }
            http.end();
        }

        free(req);

        if (xQueueSend(g_fetchRspQueue, &rsp, pdMS_TO_TICKS(2000)) != pdTRUE) {
            DBG("fetch", "Response queue full — dropping");
            free(rsp);
        }
    }

    taskShouldRun = false;
    DBG("fetch", "Fetch worker stopped");
    vTaskDelete(nullptr);
}

void fetchWorkerStop() {
    taskShouldRun = false;
}
