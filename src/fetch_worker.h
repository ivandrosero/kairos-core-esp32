#pragma once
// ─── HTTP fetch worker: Core 0 task servicing http.get/post from Elk ────────

void fetchWorkerTask(void* param);
void fetchWorkerStop();
