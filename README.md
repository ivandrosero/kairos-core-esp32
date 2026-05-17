# Kairos-Core-ESP32

Dual-core firmware for the **Paperd.ink Classic** (ESP32-WROOM, 400×300 B/W e-paper).
Scripts written in JavaScript are executed on the device via a built-in code editor,
HTTP API, WebSocket, or MQTT — with full access to the e-paper display, HTTP client,
and MQTT pub/sub from script code.

Multiple Kairos devices on the same network discover each other automatically via
ESP-NOW and form a mesh, allowing scripts to be run or saved to any peer from a
single browser tab.

> Inspired by the [famiglia-ink](https://github.com/paperdink) project.

---

## Architecture

| Core | Role | Tasks |
|------|------|-------|
| **Core 0** | Network | WiFi/LWIP, HTTP server, WebSocket client, HTTP fetch worker, MQTT client |
| **Core 1** | Engine | Scheduler, Elk JS interpreter, e-paper display, SD/LittleFS |

Cores communicate exclusively through FreeRTOS pointer queues — no shared mutable
state crosses the core boundary.

### IPC Queues

| Queue | Direction | Payload | Purpose |
|-------|-----------|---------|---------|
| `g_scriptQueue` | C0 → C1 | `ScriptJob*` | Execute JS code |
| `g_fetchReqQueue` | C1 → C0 | `FetchRequest*` | HTTP GET/POST request |
| `g_fetchRspQueue` | C0 → C1 | `FetchResponse*` | HTTP response body |
| `g_mqttInQueue` | C0 → C1 | `MqttMessage*` | Incoming MQTT messages |
| `g_mqttOutQueue` | C1 → C0 | `MqttCommand*` | Outgoing publish/subscribe |

All queued items are heap-allocated by the sender and freed by the receiver.

Each `FetchRequest`/`FetchResponse` carries a monotonic `reqId` so that
`http.get()` / `http.post()` can discard stale responses left over from a
previous timed-out execution instead of silently consuming them (the root
cause of the "display one update behind" bug).

### Startup sequence

1. Hardware init (SPI, e-paper, SD)
2. LittleFS mount
3. Load `/config.json` (or create defaults)
4. WiFi connect → NTP time sync (waits up to 5 s)
5. ESP-NOW mesh discovery
6. Create IPC queues
7. Launch Core 1 task: **Scheduler** (Elk JS + cron)
8. Launch Core 0 tasks: **WebAPI**, **Fetch Worker**, **WS Client**, **MQTT Client**

---

## Setup Mode

Kairos now has a temporary WiFi setup mode for first boot and recovery.

- If `/config.json` does not contain a saved `wifi_ssid`, the device starts its own setup network instead of trying STA mode.
- If saved WiFi credentials exist but the STA connection times out during boot, the device also falls back into setup mode automatically.
- The temporary setup network is named **`kairos-ink`**.
- When setup mode is active, the e-paper display shows the setup SSID and the config URL.
- Open `http://192.168.4.1/setup` while connected to the setup network.
- Saving the setup form writes the config to LittleFS and the device restarts automatically.

The setup page currently collects:

- Device name
- WiFi SSID
- WiFi password
- NTP server
- Time zone string

Once valid WiFi credentials are saved, setup mode is no longer used on normal boots.

## Physical Buttons

The four front buttons are initialized as active-low inputs with pull-ups enabled.
Their current behavior is hard-coded in the firmware and handled in the main loop.

| Button position | Label in code | GPIO | Current action |
|----------------|---------------|------|----------------|
| **Top** | `Menu` | `14` | Restarts the device after showing `Restarting...` on the display. |
| **Second** | `Next` | `27` | Shows current mesh status on the e-paper display, including peer count and peer IPs. |
| **Third** | `Select` | `4` | Shows device info on the e-paper display: device name, IP, free heap, uptime, and WiFi RSSI. |
| **Bottom** | `Weather` | `2` | Loads and runs the saved script named `startup`. If no such script exists, the display shows `No 'startup' script saved`. |

Notes:

- Button presses are debounced in software.
- These actions are not currently configurable from the web UI.
- Holding **Menu + Select** for about 2 seconds requests setup mode and restarts the device into the `kairos-ink` setup network.
- Holding **Menu + Select** during boot also forces setup mode.

---

## Web Editor UI

The editor is served directly from the ESP32 at `http://<device-ip>/`. It is
split into three files for maintainability, each compiled into flash as a
`PROGMEM` string:

| File | Route | Purpose |
|------|-------|---------|
| `src/ui/index.html.h` | `/` | HTML structure (tabs, toolbar, panels) |
| `src/ui/style.css.h` | `/ui/style.css` | All CSS |
| `src/ui/app.js.h` | `/ui/app.js` | All JS logic (Blockly, CodeMirror, mesh, settings) |

External dependencies (Blockly, CodeMirror) are loaded from CDN.

### Tabs

| Tab | Description |
|-----|-------------|
| **Code Editor** | CodeMirror-based JS editor with syntax highlighting. Default tab on load. Left sidebar shows mesh devices with live clocks; right panel shows the Elk JS API reference. |
| **Blockly** | Visual block-based editor. Generates Elk JS in a preview pane. Use "Code ▶ Editor" to copy generated code to the Code Editor. |
| **Settings** | Device name, NTP, WebSocket, MQTT configuration. Shows live device status (IP, heap, uptime, RSSI, time). |

### Toolbar buttons

| Button | Action |
|--------|--------|
| **▶ Run** (Ctrl+Enter) | Execute the current code on all checked devices. If only the local device is checked, runs locally via `POST /api/exec`. If multiple devices are checked, fans out via `POST /api/mesh/exec`. |
| **Save** | Save the current script to all checked devices via `POST /api/mesh/scripts`. Shows ✅ or ❌ next to each device in the sidebar. The host device is included — it also gets a checkmark. |
| **Delete** | Delete the selected script from the local device. |
| **Code ▶ Editor** | Copy Blockly-generated code into the Code Editor tab (only visible on the Blockly tab). |

### Toolbar fields

| Field | Description |
|-------|-------------|
| **Script dropdown** | Select a previously saved script to load. Shows 🧩 for Blockly scripts and 📝 for JS scripts. |
| **Script name** | Name for saving. Alphanumeric, hyphens, and underscores only. |
| **Cron** | Optional 5-field cron schedule (`MIN HR DOM MON DOW`). A human-readable hint appears as you type. |

### Devices panel

The left sidebar in the Code Editor tab shows all mesh devices:

- **Checkbox** — select which devices to target for Run/Save.
- **Device name** — from each device's config.
- **IP + live clock** — the device's NTP-synced time, ticking every second. Time is fetched from each device's `/api/status` endpoint and displayed in the device's own timezone (not the browser's).
- **Save icon** — ✅ or ❌ appears after a Save operation completes.

Devices auto-refresh every 8 seconds. Click ↻ to refresh manually.

### Output console

The bottom pane shows execution results, errors, and mesh operation status.
Drag the horizontal splitter to resize.

---

## JavaScript API

Scripts run in the [Elk](https://github.com/nicbarker/elk) embedded JS engine
(v3.0.0, 16 KB arena). Each script gets a clean engine instance.

### Display

```js
display.beginScene()               // clear buffer, prepare for drawing
display.fillScreen(color)
display.drawPixel(x, y, c)
display.drawLine(x1, y1, x2, y2, c)
display.drawRect(x, y, w, h, c)   // also fillRect
display.drawCircle(x, y, r, c)    // also fillCircle
display.drawRoundRect(x, y, w, h, r, c)  // also fillRoundRect
display.drawTriangle(x1,y1,x2,y2,x3,y3,c)  // also fillTriangle
display.setTextColor(c)
display.setCursor(x, y)
display.print(text)
display.setRotation(r)             // 0-3
display.update()                   // push buffer to e-paper
display.useTitleFont()
display.useBodyFont()
display.width()                    // 400
display.height()                   // 300
```

Constants: `BLACK` (0), `WHITE` (1).

### HTTP Fetch

```js
let r = http.get("https://api.example.com/data");
// r.status = 200, r.body = "..."

let r = http.post("https://api.example.com/submit",
                   "application/json",
                   '{"key":"value"}');
```

Requests are proxied to Core 0 via IPC. The script blocks (without spinning) until
the response arrives or a 25-second timeout expires. HTTPS is supported (no cert
validation). Each request carries a unique ID to prevent stale response mixing.

### JSON

```js
let obj = JSON.parse('{"temp":23}');
let s = JSON.stringify(obj);
```

### String

```js
let i = String.indexOf("hello world", "world");   // 6
let sub = String.substring("hello", 1, 3);         // "el"
let len = String.length("hello");                   // 5
let r = String.replace("hello world", "world", "elk");
let s = String.from(42);                            // "42"
```

### Date/Time

```js
Date.now()                // epoch seconds (from NTP)
Date.year()               // e.g. 2026
Date.month()              // 1-12
Date.day()                // 1-31
Date.hours()              // 0-23
Date.minutes()            // 0-59
Date.seconds()            // 0-59
Date.weekday()            // 0=Sun .. 6=Sat
Date.format("%H:%M")      // strftime formatting → "21:55"
Date.format("%Y-%m-%d")   // → "2026-05-13"
```

### MQTT

```js
mqtt.publish("sensors/temp", "23.5");
mqtt.subscribe("commands/#");

let msg = mqtt.receive();   // non-blocking
// msg = {topic: "commands/run", payload: "..."} or "" if empty

let ok = mqtt.connected();  // 1 or 0
```

The MQTT client auto-subscribes to `{prefix}/{device_name}/#` on connect.
Messages on `.../exec` are automatically executed as scripts.

### System

```js
sys.millis()          // uptime in ms
sys.heapFree()        // free heap bytes
sys.deepSleep(sec)    // enter deep sleep
```

### Utilities

```js
log("hello")          // serial output
delay(ms)             // max 30s, yields to RTOS
```

### Math

Full `Math` object: `sin cos tan asin acos atan atan2 abs floor ceil round trunc
sqrt pow exp log log10 sign min max random`. Constants: `PI E LN2 LN10 LOG2E
LOG10E SQRT2 SQRT1_2`.

---

## Cron Scheduling

Scripts can be assigned a 5-field cron expression when saved:

```
MIN  HR  DOM  MON  DOW
```

| Field | Range | Examples |
|-------|-------|---------|
| Minutes | 0-59 | `0`, `*/5`, `0,30` |
| Hours | 0-23 | `8`, `9-17`, `*/2` |
| Day of Month | 1-31 | `1`, `15`, `1-15` |
| Month | 1-12 | `*`, `6`, `1,6` |
| Day of Week | 0-6 (0=Sun) | `1-5`, `0,6` |

Each field supports: single values, ranges (`1-5`), steps (`*/10`, `0-30/5`),
and comma-separated combinations (`0,15,30,45`).

The scheduler checks all scripts once per minute. When a cron expression matches
the current time, the script is loaded from LittleFS and executed.

---

## Mesh Networking

Devices on the same WiFi network discover each other via ESP-NOW broadcasts.
Each device periodically announces its name, IP, and MAC address.

### How it works

1. On boot, each device starts broadcasting ESP-NOW discovery packets.
2. Peers are tracked with a 30-second timeout — if no announcement is received,
   the peer is removed.
3. The web editor shows all discovered peers in the Devices panel.
4. **Run** and **Save** operations can target any combination of devices.

### Mesh API endpoints

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/mesh/peers` | List self + all discovered peers |
| POST | `/api/mesh/exec` | Run code on selected peers (fans out HTTP requests) |
| POST | `/api/mesh/scripts` | Save a script to selected peers (fans out to each peer's `/api/scripts`) |
| GET | `/api/mesh/api?ip=x.x.x.x` | Proxy: fetch a remote peer's Elk API description |

---

## HTTP API

| Method | Path | Description |
|--------|------|-------------|
| GET | `/` | Editor UI |
| GET | `/ui/style.css` | Editor CSS |
| GET | `/ui/app.js` | Editor JavaScript |
| GET | `/api/scripts` | List saved scripts `[{name, type, size}]` |
| POST | `/api/scripts` | Save script `{name, code, type, cron}` |
| GET | `/api/scripts/:name` | Load script `{code, type, cron}` |
| DELETE | `/api/scripts/:name` | Delete script |
| POST | `/api/scripts/:name/run` | Run a saved script by name |
| POST | `/api/exec` | Execute inline code `{code}` |
| POST | `/api/display/text` | Show `{text}` on display |
| GET | `/api/config` | Get device config |
| POST | `/api/config` | Merge config `{...}` |
| GET | `/api/status` | Device status (IP, heap, uptime, RSSI, time, epoch) |
| GET | `/api/elk/api` | Structured JSON of all Elk JS bindings |
| GET | `/api/mesh/peers` | List mesh peers |
| POST | `/api/mesh/exec` | Fan-out code execution to peers |
| POST | `/api/mesh/scripts` | Fan-out script save to peers |
| GET | `/api/mesh/api?ip=` | Proxy a peer's Elk API |

---

## Configuration

All settings are persisted to `/config.json` on LittleFS and editable from the
Settings tab in the web editor.

| Setting | Default | Description |
|---------|---------|-------------|
| `device_name` | `kairos-001` | Device identifier |
| `ntp_server` | `ch.pool.ntp.org` | NTP server for time sync |
| `tz_info` | `CET-1CEST,M3.5.0,M10.5.0/3` | POSIX timezone string |
| `ws_enabled` | `false` | Connect to remote WebSocket server |
| `ws_host` / `ws_port` / `ws_path` / `ws_ssl` | — | WebSocket server details |
| `mqtt_enabled` | `false` | Connect to MQTT broker |
| `mqtt_broker` / `mqtt_port` | — / `1883` | MQTT broker address |
| `mqtt_user` / `mqtt_pass` | — | MQTT credentials (optional) |
| `mqtt_topic_prefix` | `kairos` | Base topic prefix |

WiFi credentials are set in `src/secrets.h` (not persisted to config).

---

## Project Structure

```
src/
├── main.cpp              Boot sequence, WiFi, IPC queues, task creation
├── board.cpp/.h          Hardware pin definitions, SPI init
├── config.h              Compile-time constants (ports, sizes, debug macros)
├── config_store.cpp/.h   Persistent config on LittleFS (/config.json)
├── scheduler.cpp/.h      Core 1 task: script queue + cron engine
├── elk_engine.cpp/.h     Elk JS engine: all bindings (display, http, mqtt, etc.)
├── display_utils.cpp/.h  Font presets, centered text, beginScene()
├── web_api.cpp/.h        Core 0 HTTP server: all REST routes
├── fetch_worker.cpp/.h   Core 0 task: dequeue HTTP requests from Elk, execute
├── ws_client.cpp/.h      WebSocket client (optional remote server)
├── mqtt_client.cpp/.h    MQTT client (optional broker)
├── mesh.cpp/.h           ESP-NOW peer discovery and mesh peer list
├── script_store.cpp/.h   LittleFS script storage (code + metadata)
├── ipc.h                 IPC structs and queue declarations
├── secrets.h             WiFi credentials (gitignored)
└── ui/
    ├── index.html.h      HTML structure (PROGMEM)
    ├── style.css.h       CSS (PROGMEM)
    └── app.js.h          JS: Blockly, CodeMirror, mesh, settings (PROGMEM)
```

---

## Hardware

**Paperd.ink Classic** — ESP32-WROOM-32, 4 MB flash, 320 KB RAM.

| Function | GPIO |
|----------|------|
| EPD CS / DC / Busy / Enable / Reset | 22 / 15 / 34 / 12 / 13 |
| SD Card CS | 5 (shares SPI bus with EPD) |
| Buttons (Menu / Next / Select / Weather) | 14 / 27 / 4 / 2 |

---

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload
pio run --target upload

# Monitor serial
pio device monitor
```

### Dependencies

| Library | Source |
|---------|--------|
| Elk 3.0.0 | `lib/elk/` (vendored) |
| GxEPD2 | `lib/GxEPD2/` (vendored) |
| PubSubClient | `lib/PubSubClient/` (vendored) |
| Adafruit GFX Library | PlatformIO registry |
| ArduinoJson 7.x | PlatformIO registry |
| WebSockets 2.x | PlatformIO registry |

### Elk JS Constraints

- Use `let` only (no `var`/`const`)
- Use `function` keyword (no arrow functions)
- No template literals, destructuring, async/await, classes
- No `for...in`/`for...of` — use C-style `for` loops
- No `try/catch`
- 16 KB arena, 40 expression depth, 4096 byte C stack limit
