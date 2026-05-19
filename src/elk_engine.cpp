// ─── elk_engine.cpp — Elk JS engine with display + math + sys bindings ──────
// Ported from famiglia-ink elk_engine.cpp, plus new sys.* bindings.

#include "elk_engine.h"

#include <Arduino.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <math.h>
#include <time.h>
#include "elk.h"
#include "config.h"
#include "board.h"
#include "display_utils.h"
#include "ipc.h"

// ─── Elk memory arena ───────────────────────────────────────────────────────
static uint8_t elkMem[ELK_MEM_SIZE];
static struct js *jsEngine = nullptr;

// ═══════════════════════════════════════════════════════════════════════════
//  Display bindings
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_displayWidth(struct js *js, jsval_t *, int) {
    return js_mknum(display.width());
}
static jsval_t js_displayHeight(struct js *js, jsval_t *, int) {
    return js_mknum(display.height());
}

static jsval_t js_fillScreen(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "fillScreen(color)");
    display.fillScreen((uint16_t)js_getnum(args[0]));
    return js_mkundef();
}

static jsval_t js_drawPixel(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ddd")) return js_mkerr(js, "drawPixel(x,y,c)");
    display.drawPixel((int16_t)js_getnum(args[0]),
                      (int16_t)js_getnum(args[1]),
                      (uint16_t)js_getnum(args[2]));
    return js_mkundef();
}

static jsval_t js_drawLine(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ddddd")) return js_mkerr(js, "drawLine(x1,y1,x2,y2,c)");
    display.drawLine((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                     (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                     (uint16_t)js_getnum(args[4]));
    return js_mkundef();
}

static jsval_t js_drawRect(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ddddd")) return js_mkerr(js, "drawRect(x,y,w,h,c)");
    display.drawRect((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                     (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                     (uint16_t)js_getnum(args[4]));
    return js_mkundef();
}
static jsval_t js_fillRect(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ddddd")) return js_mkerr(js, "fillRect(x,y,w,h,c)");
    display.fillRect((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                     (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                     (uint16_t)js_getnum(args[4]));
    return js_mkundef();
}

static jsval_t js_drawCircle(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dddd")) return js_mkerr(js, "drawCircle(x,y,r,c)");
    display.drawCircle((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                       (int16_t)js_getnum(args[2]), (uint16_t)js_getnum(args[3]));
    return js_mkundef();
}
static jsval_t js_fillCircle(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dddd")) return js_mkerr(js, "fillCircle(x,y,r,c)");
    display.fillCircle((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                       (int16_t)js_getnum(args[2]), (uint16_t)js_getnum(args[3]));
    return js_mkundef();
}

static jsval_t js_drawRoundRect(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dddddd")) return js_mkerr(js, "drawRoundRect(x,y,w,h,r,c)");
    display.drawRoundRect((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                          (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                          (int16_t)js_getnum(args[4]), (uint16_t)js_getnum(args[5]));
    return js_mkundef();
}
static jsval_t js_fillRoundRect(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dddddd")) return js_mkerr(js, "fillRoundRect(x,y,w,h,r,c)");
    display.fillRoundRect((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                          (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                          (int16_t)js_getnum(args[4]), (uint16_t)js_getnum(args[5]));
    return js_mkundef();
}

static jsval_t js_drawTriangle(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ddddddd")) return js_mkerr(js, "drawTriangle(7 args)");
    display.drawTriangle((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                         (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                         (int16_t)js_getnum(args[4]), (int16_t)js_getnum(args[5]),
                         (uint16_t)js_getnum(args[6]));
    return js_mkundef();
}
static jsval_t js_fillTriangle(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ddddddd")) return js_mkerr(js, "fillTriangle(7 args)");
    display.fillTriangle((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]),
                         (int16_t)js_getnum(args[2]), (int16_t)js_getnum(args[3]),
                         (int16_t)js_getnum(args[4]), (int16_t)js_getnum(args[5]),
                         (uint16_t)js_getnum(args[6]));
    return js_mkundef();
}

static jsval_t js_setTextColor(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "setTextColor(c)");
    display.setTextColor((uint16_t)js_getnum(args[0]));
    return js_mkundef();
}

static jsval_t js_setCursor(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dd")) return js_mkerr(js, "setCursor(x,y)");
    display.setCursor((int16_t)js_getnum(args[0]), (int16_t)js_getnum(args[1]));
    return js_mkundef();
}

static jsval_t js_print(struct js *js, jsval_t *args, int nargs) {
    if (nargs < 1) return js_mkundef();
    size_t len;
    char *s = js_getstr(js, args[0], &len);
    if (s) display.print(s);
    return js_mkundef();
}

static jsval_t js_setRotation(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "setRotation(r)");
    display.setRotation((uint8_t)js_getnum(args[0]));
    return js_mkundef();
}

static jsval_t js_update(struct js *js, jsval_t *, int) {
    display.display(false);
    return js_mkundef();
}

static jsval_t js_beginScene(struct js *js, jsval_t *, int) {
    beginScene();
    return js_mkundef();
}

static jsval_t js_useBodyFont(struct js *js, jsval_t *, int) {
    useBodyFont();
    return js_mkundef();
}
static jsval_t js_useTitleFont(struct js *js, jsval_t *, int) {
    useTitleFont();
    return js_mkundef();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Utility bindings
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_log(struct js *js, jsval_t *args, int nargs) {
    if (nargs < 1) return js_mkundef();
    const char *s = js_str(js, args[0]);
    if (s) { DBG("js", "%s", s); }
    return js_mkundef();
}

static jsval_t js_delay(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "delay(ms)");
    uint32_t ms = (uint32_t)js_getnum(args[0]);
    if (ms > 30000) ms = 30000;
    unsigned long deadline = millis() + ms;
    while (millis() < deadline) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    return js_mkundef();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Math bindings
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_mathSin(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.sin(x)");
    return js_mknum(sin(js_getnum(args[0])));
}
static jsval_t js_mathCos(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.cos(x)");
    return js_mknum(cos(js_getnum(args[0])));
}
static jsval_t js_mathTan(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.tan(x)");
    return js_mknum(tan(js_getnum(args[0])));
}
static jsval_t js_mathAsin(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.asin(x)");
    return js_mknum(asin(js_getnum(args[0])));
}
static jsval_t js_mathAcos(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.acos(x)");
    return js_mknum(acos(js_getnum(args[0])));
}
static jsval_t js_mathAtan(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.atan(x)");
    return js_mknum(atan(js_getnum(args[0])));
}
static jsval_t js_mathAbs(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.abs(x)");
    return js_mknum(fabs(js_getnum(args[0])));
}
static jsval_t js_mathFloor(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.floor(x)");
    return js_mknum(floor(js_getnum(args[0])));
}
static jsval_t js_mathCeil(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.ceil(x)");
    return js_mknum(ceil(js_getnum(args[0])));
}
static jsval_t js_mathRound(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.round(x)");
    return js_mknum(round(js_getnum(args[0])));
}
static jsval_t js_mathTrunc(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.trunc(x)");
    return js_mknum(trunc(js_getnum(args[0])));
}
static jsval_t js_mathSqrt(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.sqrt(x)");
    return js_mknum(sqrt(js_getnum(args[0])));
}
static jsval_t js_mathExp(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.exp(x)");
    return js_mknum(exp(js_getnum(args[0])));
}
static jsval_t js_mathLog(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.log(x)");
    return js_mknum(log(js_getnum(args[0])));
}
static jsval_t js_mathLog10(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.log10(x)");
    return js_mknum(log10(js_getnum(args[0])));
}
static jsval_t js_mathSign(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "Math.sign(x)");
    double value = js_getnum(args[0]);
    return js_mknum((value > 0) - (value < 0));
}
static jsval_t js_mathPow(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dd")) return js_mkerr(js, "Math.pow(x,y)");
    return js_mknum(pow(js_getnum(args[0]), js_getnum(args[1])));
}
static jsval_t js_mathAtan2(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "dd")) return js_mkerr(js, "Math.atan2(y,x)");
    return js_mknum(atan2(js_getnum(args[0]), js_getnum(args[1])));
}
static jsval_t js_mathMin(struct js *js, jsval_t *args, int nargs) {
    if (nargs == 0) return js_mknum(INFINITY);
    double result = INFINITY;
    for (int i = 0; i < nargs; ++i) {
        if (js_type(args[i]) != JS_NUM) return js_mkerr(js, "Math.min(...numbers)");
        double value = js_getnum(args[i]);
        if (value < result) result = value;
    }
    return js_mknum(result);
}
static jsval_t js_mathMax(struct js *js, jsval_t *args, int nargs) {
    if (nargs == 0) return js_mknum(-INFINITY);
    double result = -INFINITY;
    for (int i = 0; i < nargs; ++i) {
        if (js_type(args[i]) != JS_NUM) return js_mkerr(js, "Math.max(...numbers)");
        double value = js_getnum(args[i]);
        if (value > result) result = value;
    }
    return js_mknum(result);
}
static jsval_t js_mathRandom(struct js *js, jsval_t *, int nargs) {
    if (nargs != 0) return js_mkerr(js, "Math.random()");
    return js_mknum((double) esp_random() / (double) UINT32_MAX);
}

// ═══════════════════════════════════════════════════════════════════════════
//  sys.* bindings (NEW for kairos-core)
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_sysMillis(struct js *js, jsval_t *, int) {
    return js_mknum((double)millis());
}

static jsval_t js_sysHeapFree(struct js *js, jsval_t *, int) {
    return js_mknum((double)heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

static jsval_t js_sysDeepSleep(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "sys.deepSleep(seconds)");
    uint32_t secs = (uint32_t)js_getnum(args[0]);
    DBG("sys", "Deep sleep for %u seconds", secs);
    esp_sleep_enable_timer_wakeup((uint64_t)secs * 1000000ULL);
    esp_deep_sleep_start();
    return js_mkundef(); // never reached
}

// ═══════════════════════════════════════════════════════════════════════════
//  HTTP fetch bindings (cross-core IPC to Core 0 fetch worker)
// ═══════════════════════════════════════════════════════════════════════════

static uint32_t g_fetchIdCounter = 1;

static jsval_t js_httpGet(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "s")) return js_mkerr(js, "http.get(url)");
    size_t urlLen;
    char *url = js_getstr(js, args[0], &urlLen);
    if (!url || urlLen == 0)               return js_mkerr(js, "invalid url");
    if (urlLen >= FETCH_URL_MAX)           return js_mkerr(js, "url too long");
    if (!g_fetchReqQueue)                  return js_mkerr(js, "fetch not ready");

    FetchRequest* req = (FetchRequest*)malloc(sizeof(FetchRequest));
    if (!req) return js_mkerr(js, "out of memory");
    memset(req, 0, sizeof(FetchRequest));
    uint32_t myReqId = g_fetchIdCounter++;
    req->reqId = myReqId;
    memcpy(req->url, url, urlLen);
    req->url[urlLen] = '\0';
    strcpy(req->method, "GET");

    if (xQueueSend(g_fetchReqQueue, &req, pdMS_TO_TICKS(1000)) != pdTRUE) {
        free(req);
        return js_mkerr(js, "fetch queue full");
    }
    // req is now owned by the fetch worker — do NOT dereference it

    FetchResponse* rsp = nullptr;
    unsigned long startWait = millis();
    while (true) {
        if (xQueueReceive(g_fetchRspQueue, &rsp, pdMS_TO_TICKS(FETCH_TIMEOUT_MS)) != pdTRUE) {
            return js_mkerr(js, "fetch timeout");
        }
        if (rsp->reqId == myReqId) {
            break;
        }
        DBG("elk", "Flushed stale response id=%lu (waiting for %lu)", (unsigned long)rsp->reqId, (unsigned long)myReqId);
        free(rsp);
        rsp = nullptr;
        if (millis() - startWait > FETCH_TIMEOUT_MS) {
            return js_mkerr(js, "fetch timeout (stale loop)");
        }
    }

    jsval_t obj = js_mkobj(js);
    js_set(js, obj, "status", js_mknum(rsp->httpCode));
    js_set(js, obj, "body",   js_mkstr(js, rsp->body, rsp->bodyLen));
    free(rsp);
    return obj;
}

static jsval_t js_httpPost(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "sss")) return js_mkerr(js, "http.post(url,ct,body)");
    size_t urlLen, ctLen, bodyLen;
    char *url  = js_getstr(js, args[0], &urlLen);
    char *ct   = js_getstr(js, args[1], &ctLen);
    char *body = js_getstr(js, args[2], &bodyLen);
    if (!url || urlLen >= FETCH_URL_MAX)       return js_mkerr(js, "bad url");
    if (!ct  || ctLen  >= FETCH_CT_MAX)        return js_mkerr(js, "bad content-type");
    if (!body || bodyLen >= FETCH_REQ_BODY_MAX) return js_mkerr(js, "body too large");
    if (!g_fetchReqQueue)                      return js_mkerr(js, "fetch not ready");

    FetchRequest* req = (FetchRequest*)malloc(sizeof(FetchRequest));
    if (!req) return js_mkerr(js, "out of memory");
    memset(req, 0, sizeof(FetchRequest));
    uint32_t myReqId = g_fetchIdCounter++;
    req->reqId = myReqId;
    memcpy(req->url, url, urlLen);   req->url[urlLen] = '\0';
    strcpy(req->method, "POST");
    memcpy(req->contentType, ct, ctLen); req->contentType[ctLen] = '\0';
    memcpy(req->body, body, bodyLen);    req->body[bodyLen] = '\0';
    req->bodyLen = bodyLen;

    if (xQueueSend(g_fetchReqQueue, &req, pdMS_TO_TICKS(1000)) != pdTRUE) {
        free(req);
        return js_mkerr(js, "fetch queue full");
    }
    // req is now owned by the fetch worker — do NOT dereference it

    FetchResponse* rsp = nullptr;
    unsigned long startWait = millis();
    while (true) {
        if (xQueueReceive(g_fetchRspQueue, &rsp, pdMS_TO_TICKS(FETCH_TIMEOUT_MS)) != pdTRUE) {
            return js_mkerr(js, "fetch timeout");
        }
        if (rsp->reqId == myReqId) {
            break;
        }
        DBG("elk", "Flushed stale response id=%lu (waiting for %lu)", (unsigned long)rsp->reqId, (unsigned long)myReqId);
        free(rsp);
        rsp = nullptr;
        if (millis() - startWait > FETCH_TIMEOUT_MS) {
            return js_mkerr(js, "fetch timeout (stale loop)");
        }
    }

    jsval_t obj = js_mkobj(js);
    js_set(js, obj, "status", js_mknum(rsp->httpCode));
    js_set(js, obj, "body",   js_mkstr(js, rsp->body, rsp->bodyLen));
    free(rsp);
    return obj;
}


// ═══════════════════════════════════════════════════════════════════════════
//  JSON bindings
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_jsonStringify(struct js *js, jsval_t *args, int nargs) {
    if (nargs < 1) return js_mkundef();
    const char *str = js_str(js, args[0]);
    return js_mkstr(js, str, strlen(str));
}

static jsval_t js_jsonParse(struct js *js, jsval_t *args, int nargs) {
    if (nargs < 1) return js_mkundef();
    size_t len;
    char *str = js_getstr(js, args[0], &len);
    if (!str) return js_mkundef();

    // Wrap in parens to ensure eval evaluates it as an object expression
    char* expr = (char*)malloc(len + 3);
    if (!expr) return js_mkerr(js, "oom");
    expr[0] = '(';
    memcpy(expr + 1, str, len);
    expr[len + 1] = ')';
    expr[len + 2] = '\0';

    jsval_t res = js_eval(js, expr, len + 2);
    free(expr);
    return res;
}

// ═══════════════════════════════════════════════════════════════════════════
//  String bindings
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_strIndexOf(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "ss")) return js_mkerr(js, "String.indexOf(str, search)");
    size_t len1, len2;
    char *s1 = js_getstr(js, args[0], &len1);
    char *s2 = js_getstr(js, args[1], &len2);
    char *p = strstr(s1, s2);
    if (!p) return js_mknum(-1);
    return js_mknum(p - s1);
}

static jsval_t js_strSubstring(struct js *js, jsval_t *args, int nargs) {
    if (nargs < 2 || js_type(args[0]) != JS_STR || js_type(args[1]) != JS_NUM) return js_mkerr(js, "String.substring(str, start, [end])");
    size_t len;
    char *s = js_getstr(js, args[0], &len);
    int start = js_getnum(args[1]);
    int end = nargs > 2 && js_type(args[2]) == JS_NUM ? js_getnum(args[2]) : len;
    if (start < 0) start = 0;
    if (start > (int)len) start = len;
    if (end < 0) end = 0;
    if (end > (int)len) end = len;
    if (start > end) { int t = start; start = end; end = t; }
    return js_mkstr(js, s + start, end - start);
}

static jsval_t js_strLength(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "s")) return js_mkerr(js, "String.length(str)");
    size_t len;
    js_getstr(js, args[0], &len);
    return js_mknum(len);
}

static jsval_t js_strReplace(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "sss")) return js_mkerr(js, "String.replace(str, search, replace)");
    size_t l1, l2, l3;
    char *s = js_getstr(js, args[0], &l1);
    char *search = js_getstr(js, args[1], &l2);
    char *repl = js_getstr(js, args[2], &l3);
    char *p = strstr(s, search);
    if (!p) return js_mkstr(js, s, l1);
    
    int pre_len = p - s;
    int post_len = l1 - pre_len - l2;
    int new_len = pre_len + l3 + post_len;
    char *buf = (char *)malloc(new_len + 1);
    if (!buf) return js_mkerr(js, "oom");
    memcpy(buf, s, pre_len);
    memcpy(buf + pre_len, repl, l3);
    memcpy(buf + pre_len + l3, p + l2, post_len);
    jsval_t res = js_mkstr(js, buf, new_len);
    free(buf);
    return res;
}

static jsval_t js_strFrom(struct js *js, jsval_t *args, int nargs) {
    if (nargs < 1) return js_mkstr(js, "undefined", 9);
    const char *s = js_str(js, args[0]);
    return js_mkstr(js, s, strlen(s));
}

// ═══════════════════════════════════════════════════════════════════════════
//  Date bindings — local time from NTP-synced RTC
// ═══════════════════════════════════════════════════════════════════════════

static jsval_t js_dateNow(struct js *js, jsval_t *, int) {
    return js_mknum((double)time(nullptr));
}

static jsval_t js_dateYear(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_year + 1900);
}
static jsval_t js_dateMonth(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_mon + 1);
}
static jsval_t js_dateDay(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_mday);
}
static jsval_t js_dateHours(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_hour);
}
static jsval_t js_dateMinutes(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_min);
}
static jsval_t js_dateSeconds(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_sec);
}
static jsval_t js_dateWeekday(struct js *js, jsval_t *, int) {
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    return js_mknum(ti.tm_wday);
}
static jsval_t js_dateFormat(struct js *js, jsval_t *args, int nargs) {
    if (!js_chkargs(args, nargs, "s")) return js_mkerr(js, "Date.format(fmt)");
    size_t fmtLen;
    char *fmt = js_getstr(js, args[0], &fmtLen);
    if (!fmt || fmtLen == 0) return js_mkerr(js, "bad format");
    time_t now = time(nullptr); struct tm ti; localtime_r(&now, &ti);
    char buf[64];
    size_t n = strftime(buf, sizeof(buf), fmt, &ti);
    return js_mkstr(js, buf, n);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Engine lifecycle
// ═══════════════════════════════════════════════════════════════════════════

void elkEngineCreate() {
    if (jsEngine) {
        memset(elkMem, 0, sizeof(elkMem));
        jsEngine = nullptr;
    }
    jsEngine = js_create(elkMem, sizeof(elkMem));
    if (!jsEngine) {
        DBG("elk", "Failed to create JS engine");
        return;
    }
    js_setmaxcss(jsEngine, 4096);

    jsval_t glob = js_glob(jsEngine);

    // Color constants
    js_set(jsEngine, glob, "BLACK", js_mknum(GxEPD_BLACK));
    js_set(jsEngine, glob, "WHITE", js_mknum(GxEPD_WHITE));

    // display.* object
    jsval_t disp = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "display", disp);
    js_set(jsEngine, disp, "width",          js_mkfun(js_displayWidth));
    js_set(jsEngine, disp, "height",         js_mkfun(js_displayHeight));
    js_set(jsEngine, disp, "fillScreen",     js_mkfun(js_fillScreen));
    js_set(jsEngine, disp, "drawPixel",      js_mkfun(js_drawPixel));
    js_set(jsEngine, disp, "drawLine",       js_mkfun(js_drawLine));
    js_set(jsEngine, disp, "drawRect",       js_mkfun(js_drawRect));
    js_set(jsEngine, disp, "fillRect",       js_mkfun(js_fillRect));
    js_set(jsEngine, disp, "drawCircle",     js_mkfun(js_drawCircle));
    js_set(jsEngine, disp, "fillCircle",     js_mkfun(js_fillCircle));
    js_set(jsEngine, disp, "drawRoundRect",  js_mkfun(js_drawRoundRect));
    js_set(jsEngine, disp, "fillRoundRect",  js_mkfun(js_fillRoundRect));
    js_set(jsEngine, disp, "drawTriangle",   js_mkfun(js_drawTriangle));
    js_set(jsEngine, disp, "fillTriangle",   js_mkfun(js_fillTriangle));
    js_set(jsEngine, disp, "setTextColor",   js_mkfun(js_setTextColor));
    js_set(jsEngine, disp, "setCursor",      js_mkfun(js_setCursor));
    js_set(jsEngine, disp, "print",          js_mkfun(js_print));
    js_set(jsEngine, disp, "setRotation",    js_mkfun(js_setRotation));
    js_set(jsEngine, disp, "update",         js_mkfun(js_update));
    js_set(jsEngine, disp, "beginScene",     js_mkfun(js_beginScene));
    js_set(jsEngine, disp, "useBodyFont",    js_mkfun(js_useBodyFont));
    js_set(jsEngine, disp, "useTitleFont",   js_mkfun(js_useTitleFont));

    // Globals
    js_set(jsEngine, glob, "log",   js_mkfun(js_log));
    js_set(jsEngine, glob, "delay", js_mkfun(js_delay));

    // Math.* object
    jsval_t math = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "Math", math);
    js_set(jsEngine, math, "abs",    js_mkfun(js_mathAbs));
    js_set(jsEngine, math, "acos",   js_mkfun(js_mathAcos));
    js_set(jsEngine, math, "asin",   js_mkfun(js_mathAsin));
    js_set(jsEngine, math, "atan",   js_mkfun(js_mathAtan));
    js_set(jsEngine, math, "atan2",  js_mkfun(js_mathAtan2));
    js_set(jsEngine, math, "ceil",   js_mkfun(js_mathCeil));
    js_set(jsEngine, math, "sin",    js_mkfun(js_mathSin));
    js_set(jsEngine, math, "cos",    js_mkfun(js_mathCos));
    js_set(jsEngine, math, "tan",    js_mkfun(js_mathTan));
    js_set(jsEngine, math, "exp",    js_mkfun(js_mathExp));
    js_set(jsEngine, math, "floor",  js_mkfun(js_mathFloor));
    js_set(jsEngine, math, "log",    js_mkfun(js_mathLog));
    js_set(jsEngine, math, "log10",  js_mkfun(js_mathLog10));
    js_set(jsEngine, math, "max",    js_mkfun(js_mathMax));
    js_set(jsEngine, math, "min",    js_mkfun(js_mathMin));
    js_set(jsEngine, math, "pow",    js_mkfun(js_mathPow));
    js_set(jsEngine, math, "random", js_mkfun(js_mathRandom));
    js_set(jsEngine, math, "round",  js_mkfun(js_mathRound));
    js_set(jsEngine, math, "sign",   js_mkfun(js_mathSign));
    js_set(jsEngine, math, "sqrt",   js_mkfun(js_mathSqrt));
    js_set(jsEngine, math, "trunc",  js_mkfun(js_mathTrunc));
    js_set(jsEngine, math, "E",      js_mknum(M_E));
    js_set(jsEngine, math, "LN2",    js_mknum(M_LN2));
    js_set(jsEngine, math, "LN10",   js_mknum(M_LN10));
    js_set(jsEngine, math, "LOG2E",  js_mknum(M_LOG2E));
    js_set(jsEngine, math, "LOG10E", js_mknum(M_LOG10E));
    js_set(jsEngine, math, "PI",     js_mknum(PI));
    js_set(jsEngine, math, "SQRT1_2",js_mknum(M_SQRT1_2));
    js_set(jsEngine, math, "SQRT2",  js_mknum(M_SQRT2));

    // sys.* object
    jsval_t sys = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "sys", sys);
    js_set(jsEngine, sys, "millis",    js_mkfun(js_sysMillis));
    js_set(jsEngine, sys, "heapFree",  js_mkfun(js_sysHeapFree));
    js_set(jsEngine, sys, "deepSleep", js_mkfun(js_sysDeepSleep));

    // http.* object
    jsval_t http = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "http", http);
    js_set(jsEngine, http, "get",  js_mkfun(js_httpGet));
    js_set(jsEngine, http, "post", js_mkfun(js_httpPost));


    // JSON object
    jsval_t jsonObj = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "JSON", jsonObj);
    js_set(jsEngine, jsonObj, "stringify", js_mkfun(js_jsonStringify));
    js_set(jsEngine, jsonObj, "parse",     js_mkfun(js_jsonParse));

    // String utility object
    jsval_t strObj = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "String", strObj);
    js_set(jsEngine, strObj, "indexOf",   js_mkfun(js_strIndexOf));
    js_set(jsEngine, strObj, "substring", js_mkfun(js_strSubstring));
    js_set(jsEngine, strObj, "length",    js_mkfun(js_strLength));
    js_set(jsEngine, strObj, "replace",   js_mkfun(js_strReplace));
    js_set(jsEngine, strObj, "from",      js_mkfun(js_strFrom));

    // Date.* object
    jsval_t dateObj = js_mkobj(jsEngine);
    js_set(jsEngine, glob, "Date", dateObj);
    js_set(jsEngine, dateObj, "now",     js_mkfun(js_dateNow));
    js_set(jsEngine, dateObj, "year",    js_mkfun(js_dateYear));
    js_set(jsEngine, dateObj, "month",   js_mkfun(js_dateMonth));
    js_set(jsEngine, dateObj, "day",     js_mkfun(js_dateDay));
    js_set(jsEngine, dateObj, "hours",   js_mkfun(js_dateHours));
    js_set(jsEngine, dateObj, "minutes", js_mkfun(js_dateMinutes));
    js_set(jsEngine, dateObj, "seconds", js_mkfun(js_dateSeconds));
    js_set(jsEngine, dateObj, "weekday", js_mkfun(js_dateWeekday));
    js_set(jsEngine, dateObj, "format",  js_mkfun(js_dateFormat));

    size_t total, lwm, css;
    js_stats(jsEngine, &total, &lwm, &css);
    DBG("elk", "Engine ready — total %u, free %u, css %u",
        (unsigned)total, (unsigned)lwm, (unsigned)css);
}

void elkEngineDestroy() {
    if (jsEngine) {
        memset(elkMem, 0, sizeof(elkMem));
        jsEngine = nullptr;
    }
}

bool elkEngineExec(const char* code, size_t len, String* errorOut) {
    if (!jsEngine) {
        elkEngineCreate();
    }
    if (!jsEngine) {
        if (errorOut) *errorOut = "Failed to create JS engine";
        return false;
    }

    DBG("elk", "Exec %u bytes", (unsigned)len);

    // Drain any stale fetch responses/requests left over from a previous
    // execution that timed-out on the IPC side.  Without this, the next
    // http.get()/post() picks up the old response and every subsequent
    // call is shifted by one — the peer display appears to "not update."
    {
        FetchResponse* stale = nullptr;
        while (xQueueReceive(g_fetchRspQueue, &stale, 0) == pdTRUE) {
            if (stale) { DBG("elk", "Flushed stale fetch response"); free(stale); }
        }
        FetchRequest* staleReq = nullptr;
        while (xQueueReceive(g_fetchReqQueue, &staleReq, 0) == pdTRUE) {
            if (staleReq) { DBG("elk", "Flushed stale fetch request"); free(staleReq); }
        }
    }

    display.setRotation(0);
    display.setFullWindow();

    jsval_t result = js_eval(jsEngine, code, len);
    int rtype = js_type(result);

    bool ok = (rtype != JS_ERR);
    if (!ok) {
        const char *err = js_str(jsEngine, result);
        DBG("elk", "ERROR: %s", err);
        if (errorOut) *errorOut = err;
    } else {
        DBG("elk", "OK: %s", js_str(jsEngine, result));
    }

    size_t total, lwm, css;
    js_stats(jsEngine, &total, &lwm, &css);
    DBG("elk", "Post-exec — total %u, lwm %u, css %u",
        (unsigned)total, (unsigned)lwm, (unsigned)css);

    // Recreate engine so each script starts clean
    memset(elkMem, 0, sizeof(elkMem));
    jsEngine = nullptr;
    elkEngineCreate();

    return ok;
}

struct js* elkEngineGet() {
    return jsEngine;
}
