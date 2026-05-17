#pragma once
// ─── WebSocket client: connect to remote server to receive scripts ──────────

void wsClientTask(void* param);
void wsClientStop();
bool wsClientIsConnected();
