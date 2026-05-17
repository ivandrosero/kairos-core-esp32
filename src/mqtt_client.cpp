// ─── mqtt_client.cpp — Core 0 task: PubSubClient connection + IPC ───────────
// Auto-subscribes to {prefix}/{device_name}/# on connect.
// Messages on .../exec are routed as ScriptJobs to g_scriptQueue.
// All other messages are pushed to g_mqttInQueue for Elk mqtt.receive().
// Outgoing publishes/subscribes arrive from g_mqttOutQueue (Elk mqtt.publish).

#include "mqtt_client.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "config_store.h"
#include "ipc.h"

static WiFiClient mqttWifi;
static PubSubClient mqttClient(mqttWifi);
static volatile bool mqttConnected  = false;
static volatile bool mqttShouldRun  = false;
static String mqttBaseTopic;              // e.g. "kairos/kairos-001"

// ─── Incoming message callback (runs in mqttClient.loop() context) ──────────
static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr(topic);
    String execTopic = mqttBaseTopic + "/exec";

    if (topicStr == execTopic) {
        // Script execution request → ScriptJob → g_scriptQueue
        if (length > 0 && length < SCRIPT_CODE_MAX) {
            ScriptJob* job = (ScriptJob*)malloc(sizeof(ScriptJob));
            if (job) {
                strncpy(job->name, "__mqtt__", SCRIPT_NAME_MAX - 1);
                job->name[SCRIPT_NAME_MAX - 1] = '\0';
                memcpy(job->code, payload, length);
                job->code[length] = '\0';
                job->codeLen = length;
                if (xQueueSend(g_scriptQueue, &job, 0) != pdTRUE) {
                    DBG("mqtt", "Script queue full, dropping");
                    free(job);
                }
            }
        }
        return;
    }

    // Regular message → MqttMessage → g_mqttInQueue
    MqttMessage* msg = (MqttMessage*)malloc(sizeof(MqttMessage));
    if (!msg) return;

    size_t topicLen = topicStr.length();
    if (topicLen >= MQTT_TOPIC_MAX) topicLen = MQTT_TOPIC_MAX - 1;
    memcpy(msg->topic, topic, topicLen);
    msg->topic[topicLen] = '\0';

    size_t payloadLen = length;
    if (payloadLen >= MQTT_PAYLOAD_MAX) payloadLen = MQTT_PAYLOAD_MAX - 1;
    memcpy(msg->payload, payload, payloadLen);
    msg->payload[payloadLen] = '\0';
    msg->payloadLen = payloadLen;

    if (xQueueSend(g_mqttInQueue, &msg, 0) != pdTRUE) {
        free(msg);
    }
}

// ─── Reconnect logic ───────────────────────────────────────────────────────
static void mqttReconnect() {
    KairosConfig& cfg = configGet();
    String clientId = "kairos-" + cfg.device_name;

    bool ok;
    if (cfg.mqtt_user.length() > 0) {
        ok = mqttClient.connect(clientId.c_str(),
                                cfg.mqtt_user.c_str(),
                                cfg.mqtt_pass.c_str());
    } else {
        ok = mqttClient.connect(clientId.c_str());
    }

    if (ok) {
        mqttConnected = true;
        DBG("mqtt", "Connected to broker");
        String wildcard = mqttBaseTopic + "/#";
        mqttClient.subscribe(wildcard.c_str());
        DBG("mqtt", "Subscribed to %s", wildcard.c_str());
    } else {
        mqttConnected = false;
        DBG("mqtt", "Connect failed, rc=%d", mqttClient.state());
    }
}

// ─── Task entry point ──────────────────────────────────────────────────────
void mqttClientTask(void* param) {
    (void)param;
    mqttShouldRun = true;

    KairosConfig& cfg = configGet();
    if (!cfg.mqtt_enabled || cfg.mqtt_broker.length() == 0) {
        DBG("mqtt", "MQTT disabled or no broker configured");
        mqttShouldRun = false;
        vTaskDelete(nullptr);
        return;
    }

    mqttBaseTopic = cfg.mqtt_topic_prefix + "/" + cfg.device_name;

    mqttClient.setServer(cfg.mqtt_broker.c_str(), cfg.mqtt_port);
    mqttClient.setBufferSize(1280);
    mqttClient.setCallback(mqttCallback);

    DBG("mqtt", "Starting, broker %s:%d, base topic %s (Core %d)",
        cfg.mqtt_broker.c_str(), cfg.mqtt_port,
        mqttBaseTopic.c_str(), xPortGetCoreID());

    unsigned long lastReconnect = 0;

    while (mqttShouldRun) {
        // Reconnect with back-off
        if (!mqttClient.connected()) {
            mqttConnected = false;
            unsigned long now = millis();
            if (now - lastReconnect > 5000) {
                lastReconnect = now;
                mqttReconnect();
            }
        } else {
            mqttClient.loop();
        }

        // Drain outgoing command queue (publish / subscribe / unsubscribe)
        MqttCommand* cmd = nullptr;
        while (xQueueReceive(g_mqttOutQueue, &cmd, 0) == pdTRUE) {
            if (!cmd) continue;
            if (mqttClient.connected()) {
                switch (cmd->type) {
                    case MQTT_CMD_PUBLISH:
                        mqttClient.publish(cmd->topic,
                                           (uint8_t*)cmd->payload,
                                           cmd->payloadLen);
                        break;
                    case MQTT_CMD_SUBSCRIBE:
                        mqttClient.subscribe(cmd->topic);
                        DBG("mqtt", "Subscribed to %s", cmd->topic);
                        break;
                    case MQTT_CMD_UNSUBSCRIBE:
                        mqttClient.unsubscribe(cmd->topic);
                        break;
                }
            }
            free(cmd);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    mqttClient.disconnect();
    mqttConnected = false;
    mqttShouldRun = false;
    DBG("mqtt", "MQTT task stopped");
    vTaskDelete(nullptr);
}

void mqttClientStop() {
    mqttShouldRun = false;
}

bool mqttClientIsConnected() {
    return mqttConnected;
}
