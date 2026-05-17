#pragma once
// ─── HTTP API + embedded WS server (Core 0 task) ────────────────────────────

void webApiTask(void* param);
void webApiStop();
bool webApiIsRunning();
void webApiSetSetupMode(bool enabled);
bool webApiIsSetupMode();
bool webApiConsumeRestartRequest();
