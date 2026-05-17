#pragma once
// ─── MQTT client: Core 0 task, PubSubClient wrapper ────────────────────────

void mqttClientTask(void* param);
void mqttClientStop();
bool mqttClientIsConnected();
