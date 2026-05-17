#pragma once
// ─── Scheduler: Core 1 task that executes scripts from the queue ────────────

void schedulerTask(void* param);

#include <stddef.h>
bool enqueueCode(const char* name, const char* code, size_t len);
