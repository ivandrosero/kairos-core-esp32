#pragma once
// ─── Shared Elk JS engine with display + math + sys bindings ────────────────

#include <Arduino.h>
#include "elk.h"

void elkEngineCreate();
void elkEngineDestroy();
bool elkEngineExec(const char* code, size_t len, String* errorOut = nullptr);
struct js* elkEngineGet();
