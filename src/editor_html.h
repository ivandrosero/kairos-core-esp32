#pragma once
// ─── Embedded HTML for the Kairos editor served from the device ─────────────
// Three tabs: Blockly visual editor, Code (CodeMirror) editor, Settings.
// Ported from famiglia-ink editor_html.h, adapted for kairos-core API paths.

static const char EDITOR_HTML[] PROGMEM = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
  <title>Kairos Editor</title>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.16/codemirror.min.css"/>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.16/theme/monokai.min.css"/>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    html, body { height: 100%; font-family: -apple-system, BlinkMacSystemFont, sans-serif; background: #1e1e1e; color: #ccc; display: flex; flex-direction: column; overflow: hidden; }
    .toolbar { display: flex; align-items: center; gap: 8px; padding: 8px 12px; background: #2d2d2d; border-bottom: 1px solid #444; flex-wrap: wrap; flex-shrink: 0; }
    .toolbar select, .toolbar input, .toolbar button { font-size: 13px; padding: 4px 8px; border-radius: 4px; border: 1px solid #555; background: #3a3a3a; color: #eee; }
    .toolbar button { cursor: pointer; font-weight: 600; }
    .toolbar button:hover { background: #505050; }
    .toolbar button.primary { background: #2a7a2a; border-color: #3a9a3a; }
    .toolbar button.primary:hover { background: #3a9a3a; }
    .toolbar button.danger { background: #7a2a2a; border-color: #9a3a3a; }
    .toolbar button.danger:hover { background: #9a3a3a; }
    .toolbar button.accent { background: #2a5a7a; border-color: #3a7a9a; }
    .toolbar button.accent:hover { background: #3a7a9a; }
    .toolbar label { font-size: 12px; color: #aaa; }
    .toolbar .spacer { flex: 1; }
    .toolbar .status { font-size: 11px; padding: 3px 8px; border-radius: 3px; }
    .status.ok { background: #1a4a1a; color: #6f6; }
    .status.err { background: #4a1a1a; color: #f66; }
    .status.info { background: #1a3a4a; color: #6cf; }
    #save-name { width: 140px; }
    .tab-bar { display: flex; background: #2d2d2d; border-bottom: 2px solid #444; flex-shrink: 0; }
    .tab-bar button { padding: 7px 22px; background: transparent; border: none; border-bottom: 2px solid transparent; color: #888; font-size: 13px; font-weight: 600; cursor: pointer; margin-bottom: -2px; }
    .tab-bar button:hover { color: #ccc; background: #333; }
    .tab-bar button.active { color: #eee; border-bottom-color: #6cf; background: #1e1e1e; }
    .tab-content { display: none; flex: 1; overflow: hidden; }
    .tab-content.active { display: flex; }
    .blockly-layout { display: flex; flex: 1; overflow: hidden; }
    #blockly-area { flex: 1; position: relative; min-width: 200px; }
    #blockly-div { position: absolute; top: 0; left: 0; right: 0; bottom: 0; }
    .code-preview { width: 300px; min-width: 120px; display: flex; flex-direction: column; background: #252525; border-left: 1px solid #444; }
    .code-preview-hdr { display: flex; align-items: center; justify-content: space-between; padding: 6px 12px; background: #2d2d2d; border-bottom: 1px solid #444; flex-shrink: 0; }
    .code-preview-hdr h4 { color: #eee; font-size: 13px; margin: 0; }
    .code-preview-hdr button { font-size: 11px; padding: 2px 8px; border-radius: 3px; border: 1px solid #555; background: #3a3a3a; color: #ccc; cursor: pointer; }
    .code-preview-hdr button:hover { background: #505050; }
    .code-preview pre { flex: 1; overflow: auto; padding: 10px 12px; margin: 0; font-family: 'SF Mono', Monaco, Menlo, monospace; font-size: 12px; color: #8cf; white-space: pre-wrap; line-height: 1.5; }
    .code-layout { display: flex; flex: 1; overflow: hidden; }
    .left-pane { display: flex; flex-direction: column; flex: 1; min-width: 200px; overflow: hidden; }
    .editor-wrap { flex: 1; overflow: hidden; }
    .CodeMirror { height: 100%; font-size: 13px; }
    .info-panel { width: 400px; min-width: 150px; overflow-y: auto; padding: 12px 14px; background: #252525; font-size: 12px; color: #aaa; line-height: 1.6; }
    .info-panel h4 { color: #eee; margin: 12px 0 6px 0; font-size: 13px; }
    .info-panel h4:first-child { margin-top: 0; }
    .info-panel ul { margin: 0 0 0 16px; padding: 0; }
    .info-panel li { margin: 2px 0; }
    .info-panel code { color: #8cf; font-size: 11px; }
    .devices-panel { width: 180px; min-width: 120px; overflow-y: auto; padding: 8px 10px; background: #252525; border-right: 1px solid #444; font-size: 12px; color: #ccc; flex-shrink: 0; }
    .devices-panel h4 { color: #eee; margin: 0 0 8px 0; font-size: 13px; display: flex; align-items: center; justify-content: space-between; }
    .devices-panel .refresh-btn { background: none; border: none; color: #6cf; cursor: pointer; font-size: 14px; padding: 2px; }
    .device-item { display: flex; align-items: center; gap: 6px; padding: 4px 2px; border-radius: 3px; cursor: pointer; }
    .device-item:hover { background: #333; }
    .device-item input { margin: 0; accent-color: #6cf; }
    .device-item .dev-name { font-weight: 600; color: #eee; font-size: 11px; }
    .device-item .dev-ip { color: #888; font-size: 10px; }
    .device-item .dev-self { color: #6f6; font-size: 9px; font-weight: 600; }
    .device-item .dev-remote { color: #fc6; font-size: 9px; }
    .vsplit { flex: 0 0 6px; background: #444; cursor: col-resize; user-select: none; -webkit-user-select: none; }
    .vsplit:hover { background: #666; }
    .hsplit { flex: 0 0 6px; background: #444; cursor: row-resize; user-select: none; -webkit-user-select: none; }
    .hsplit:hover { background: #666; }
    .output { height: 120px; min-height: 40px; overflow-y: auto; background: #111; padding: 8px; font-family: monospace; font-size: 12px; white-space: pre-wrap; flex-shrink: 0; border-top: 1px solid #444; }
    .output .err { color: #f66; }
    .output .ok { color: #6f6; }
    .output .log { color: #ccc; }
    .settings-layout { flex: 1; overflow-y: auto; padding: 24px; max-width: 600px; }
    .settings-layout h3 { color: #eee; margin: 0 0 16px 0; }
    .settings-layout .field { margin-bottom: 14px; }
    .settings-layout label { display: block; font-size: 12px; color: #aaa; margin-bottom: 4px; }
    .settings-layout input[type=text], .settings-layout input[type=number], .settings-layout input[type=password] {
      width: 100%; padding: 6px 10px; border-radius: 4px; border: 1px solid #555; background: #3a3a3a; color: #eee; font-size: 13px; }
    .settings-layout .checkbox { display: flex; align-items: center; gap: 8px; }
    .settings-layout button.primary { margin-top: 12px; }
    .settings-layout .status-info { margin-top: 20px; padding: 12px; background: #252525; border-radius: 6px; font-size: 12px; }
    .settings-layout .status-info div { margin: 4px 0; }
    
    @media (max-width: 700px) {
      .code-layout { flex-direction: column; }
      .blockly-layout { flex-direction: column; }
      .info-panel { width: auto !important; max-height: 200px; }
      .code-preview { width: auto !important; max-height: 200px; }
      .vsplit { display: none; }
      .toolbar { gap: 4px; }
      #save-name { width: 100px; }
    }
  </style>
</head>
<body>
  <div class="toolbar">
    <select id="scripts-dropdown"><option value="">-- new script --</option></select>
    <input id="save-name" type="text" placeholder="script name"/>
    <input id="save-cron" type="text" placeholder="cron (min hr * * *)" style="width:130px;" title="Cron schedule: MIN HOUR DOM MON DOW" />
    <span id="cron-hint" style="font-size:10px;color:#8cf;max-width:180px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;"></span>
    <button class="primary" id="btn-run" title="Ctrl+Enter">&#9654; Run</button>
    <button id="btn-save">Save</button>
    <button class="danger" id="btn-delete">Delete</button>
    <button class="accent" id="btn-copy" title="Copy generated Blockly code into Code Editor">Code &#9654; Editor</button>
    <div class="spacer"></div>
    <span id="status" class="status info">ready</span>
  </div>
  <div class="tab-bar">
    <button class="tab-btn active" data-tab="blockly">Blockly</button>
    <button class="tab-btn" data-tab="code">Code Editor</button>
    <button class="tab-btn" data-tab="settings">Settings</button>
  </div>
  <div id="tab-blockly" class="tab-content active">
    <div class="blockly-layout">
      <div id="blockly-area"><div id="blockly-div"></div></div>
      <div class="vsplit" id="vsplit-blockly"></div>
      <div class="code-preview" id="code-preview-panel">
        <div class="code-preview-hdr">
          <h4>Generated Elk JS</h4>
          <button id="btn-copy-code" title="Copy to clipboard">&#128203; Copy</button>
        </div>
        <pre id="generated-code">// Drag blocks to generate code</pre>
      </div>
    </div>
  </div>
  <div id="tab-code" class="tab-content">
    <div class="code-layout">
      <div class="devices-panel" id="devices-panel">
        <h4>Devices <button class="refresh-btn" id="btn-refresh-peers" title="Refresh">&#8635;</button></h4>
        <div id="devices-list">Loading...</div>
      </div>
      <div class="left-pane" id="left-pane">
        <div class="editor-wrap"><textarea id="code">// Kairos Elk JS
// Press Ctrl+Enter or click Run to execute

display.beginScene();
display.useTitleFont();
display.setCursor(10, 40);
display.print("Hello from Kairos!");
display.useBodyFont();
display.setCursor(10, 80);
display.print("400x300 e-paper");
display.update();
</textarea></div>
      </div>
      <div class="vsplit" id="vsplit-code"></div>
      <div class="info-panel" id="info-panel">
        <h4>Display</h4>
        <ul>
          <li><code>display.beginScene()</code></li>
          <li><code>display.fillScreen(color)</code></li>
          <li><code>display.drawPixel(x, y, c)</code></li>
          <li><code>display.drawLine(x1, y1, x2, y2, c)</code></li>
          <li><code>display.drawRect(x, y, w, h, c)</code></li>
          <li><code>display.fillRect(x, y, w, h, c)</code></li>
          <li><code>display.drawCircle(x, y, r, c)</code></li>
          <li><code>display.fillCircle(x, y, r, c)</code></li>
          <li><code>display.drawRoundRect(x, y, w, h, r, c)</code></li>
          <li><code>display.fillRoundRect(x, y, w, h, r, c)</code></li>
          <li><code>display.drawTriangle(x1,y1,x2,y2,x3,y3,c)</code></li>
          <li><code>display.fillTriangle(x1,y1,x2,y2,x3,y3,c)</code></li>
          <li><code>display.setTextColor(c)</code></li>
          <li><code>display.setCursor(x, y)</code></li>
          <li><code>display.print(text)</code></li>
          <li><code>display.setRotation(r)</code></li>
          <li><code>display.update()</code></li>
          <li><code>display.width()</code> / <code>display.height()</code></li>
          <li><code>display.useBodyFont()</code> / <code>display.useTitleFont()</code></li>
        </ul>
        <h4>Constants</h4>
        <ul><li><code>BLACK</code> (0), <code>WHITE</code> (1)</li></ul>
        <h4>Utilities</h4>
        <ul>
          <li><code>log(msg)</code> — serial print</li>
          <li><code>delay(ms)</code> — max 30s</li>
        </ul>
        <h4>System</h4>
        <ul>
          <li><code>sys.millis()</code> — uptime ms</li>
          <li><code>sys.heapFree()</code> — free bytes</li>
          <li><code>sys.deepSleep(sec)</code></li>
        </ul>
        <h4>HTTP Fetch</h4>
        <ul>
          <li><code>http.get(url)</code> — {status, body}</li>
          <li><code>http.post(url, contentType, body)</code></li>
        </ul>

        <h4>Date/Time</h4>
        <ul>
          <li><code>Date.now()</code> — epoch seconds</li>
          <li><code>Date.year()</code> <code>Date.month()</code> <code>Date.day()</code></li>
          <li><code>Date.hours()</code> <code>Date.minutes()</code> <code>Date.seconds()</code></li>
          <li><code>Date.weekday()</code> — 0=Sun..6=Sat</li>
          <li><code>Date.format(fmt)</code> — strftime format</li>
          <li>e.g. <code>Date.format("%H:%M")</code> → "21:55"</li>
          <li>e.g. <code>Date.format("%Y-%m-%d")</code> → "2026-05-13"</li>
        </ul>
        <h4>Math (Math.xxx)</h4>
        <ul>
          <li><code>Math.abs ceil floor round trunc sign</code></li>
          <li><code>Math.sin cos tan asin acos atan atan2</code></li>
          <li><code>Math.sqrt pow exp log log10</code></li>
          <li><code>Math.min(a,b) max(a,b) random()</code></li>
          <li><code>Math.PI E LN2 LN10 LOG2E LOG10E SQRT2 SQRT1_2</code></li>
        </ul>
        <h4>Elk JS Notes</h4>
        <ul>
          <li>Use <code>let</code> (not var/const)</li>
          <li>Use <code>function</code> (not arrow =>)</li>
          <li>No template literals</li>
          <li>No for..of / for..in</li>
          <li>No try/catch</li>
        </ul>
      </div>
    </div>
  </div>
  <div id="tab-settings" class="tab-content">
    <div class="settings-layout">
      <h3>Kairos Settings</h3>
      <div class="field">
        <label>Device Name</label>
        <input type="text" id="cfg-device-name" placeholder="kairos-001"/>
      </div>
      <h3>WebSocket Client</h3>
      <div class="field checkbox">
        <input type="checkbox" id="cfg-ws-enabled"/>
        <label for="cfg-ws-enabled">Enable WS client (connect to remote server)</label>
      </div>
      <div class="field">
        <label>WS Host</label>
        <input type="text" id="cfg-ws-host" placeholder="your-server.example.com"/>
      </div>
      <div class="field">
        <label>WS Port</label>
        <input type="number" id="cfg-ws-port" value="443"/>
      </div>
      <div class="field">
        <label>WS Path</label>
        <input type="text" id="cfg-ws-path" placeholder="/kairos/{device_id}"/>
      </div>
      <div class="field checkbox">
        <input type="checkbox" id="cfg-ws-ssl" checked/>
        <label for="cfg-ws-ssl">Use SSL (wss://)</label>
      </div>
      <h3>Time Sync (NTP)</h3>
      <div class="field">
        <label>NTP Server</label>
        <input type="text" id="cfg-ntp-server" placeholder="pool.ntp.org" />
      </div>
      <div class="field">
        <label>Timezone (POSIX TZ string)</label>
        <input type="text" id="cfg-tz-info" placeholder="EST5EDT,M3.2.0,M11.1.0" />
      </div>

      <button class="primary" id="btn-cfg-save">Save &amp; Apply</button>
      <span id="cfg-status" class="status info" style="margin-left:12px;"></span>
      <div class="status-info" id="device-status">
        <div><strong>Device Status</strong></div>
        <div id="st-ip">IP: —</div>
        <div id="st-heap">Heap: —</div>
        <div id="st-uptime">Uptime: —</div>
        <div id="st-ws">WS Client: —</div>

        <div id="st-rssi">WiFi RSSI: —</div>
        <div id="st-time">Device Time: —</div>
      </div>
    </div>
  </div>
  <div class="hsplit" id="hsplit"></div>
  <div class="output" id="output"></div>

  <script src="https://unpkg.com/blockly/blockly_compressed.js"></script>
  <script src="https://unpkg.com/blockly/blocks_compressed.js"></script>
  <script src="https://unpkg.com/blockly/javascript_compressed.js"></script>
  <script src="https://unpkg.com/blockly/msg/en.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.16/codemirror.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.16/mode/javascript/javascript.min.js"></script>
  <script>
(function() {
  var DISPLAY_HUE = 210, CONST_HUE = 20, UTIL_HUE = 160;

  Blockly.defineBlocksWithJsonArray([
    { type: 'display_begin_scene', message0: 'begin scene', previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_update', message0: 'update display', previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_fill_screen', message0: 'fill screen %1', args0: [{ type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_use_title_font', message0: 'use title font', previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_use_body_font', message0: 'use body font', previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_set_text_color', message0: 'set text color %1', args0: [{ type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_set_cursor', message0: 'set cursor x %1 y %2', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_print', message0: 'print %1', args0: [{ type: 'input_value', name: 'TEXT', check: 'String' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_set_rotation', message0: 'set rotation %1', args0: [{ type: 'field_dropdown', name: 'R', options: [['0\u00B0','0'],['90\u00B0','1'],['180\u00B0','2'],['270\u00B0','3']] }], previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_width', message0: 'display width', output: 'Number', colour: DISPLAY_HUE },
    { type: 'display_height', message0: 'display height', output: 'Number', colour: DISPLAY_HUE },
    { type: 'display_draw_pixel', message0: 'draw pixel x %1 y %2 color %3', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_draw_line', message0: 'draw line x1 %1 y1 %2 x2 %3 y2 %4 color %5', args0: [{ type: 'input_value', name: 'X1', check: 'Number' }, { type: 'input_value', name: 'Y1', check: 'Number' }, { type: 'input_value', name: 'X2', check: 'Number' }, { type: 'input_value', name: 'Y2', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_draw_rect', message0: 'draw rect x %1 y %2 w %3 h %4 color %5', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'W', check: 'Number' }, { type: 'input_value', name: 'H', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_fill_rect', message0: 'fill rect x %1 y %2 w %3 h %4 color %5', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'W', check: 'Number' }, { type: 'input_value', name: 'H', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_draw_circle', message0: 'draw circle x %1 y %2 r %3 color %4', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'R', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_fill_circle', message0: 'fill circle x %1 y %2 r %3 color %4', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'R', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_draw_round_rect', message0: 'draw rounded rect x %1 y %2 w %3 h %4 corner %5 color %6', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'W', check: 'Number' }, { type: 'input_value', name: 'H', check: 'Number' }, { type: 'input_value', name: 'CR', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_fill_round_rect', message0: 'fill rounded rect x %1 y %2 w %3 h %4 corner %5 color %6', args0: [{ type: 'input_value', name: 'X', check: 'Number' }, { type: 'input_value', name: 'Y', check: 'Number' }, { type: 'input_value', name: 'W', check: 'Number' }, { type: 'input_value', name: 'H', check: 'Number' }, { type: 'input_value', name: 'CR', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_draw_triangle', message0: 'draw triangle %1 %2 %3 %4 %5 %6 color %7', args0: [{ type: 'input_value', name: 'X1', check: 'Number' }, { type: 'input_value', name: 'Y1', check: 'Number' }, { type: 'input_value', name: 'X2', check: 'Number' }, { type: 'input_value', name: 'Y2', check: 'Number' }, { type: 'input_value', name: 'X3', check: 'Number' }, { type: 'input_value', name: 'Y3', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: false, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'display_fill_triangle', message0: 'fill triangle %1 %2 %3 %4 %5 %6 color %7', args0: [{ type: 'input_value', name: 'X1', check: 'Number' }, { type: 'input_value', name: 'Y1', check: 'Number' }, { type: 'input_value', name: 'X2', check: 'Number' }, { type: 'input_value', name: 'Y2', check: 'Number' }, { type: 'input_value', name: 'X3', check: 'Number' }, { type: 'input_value', name: 'Y3', check: 'Number' }, { type: 'input_value', name: 'COLOR', check: 'Number' }], inputsInline: false, previousStatement: null, nextStatement: null, colour: DISPLAY_HUE },
    { type: 'color_constant', message0: '%1', args0: [{ type: 'field_dropdown', name: 'COLOR', options: [['BLACK','BLACK'],['WHITE','WHITE']] }], output: 'Number', colour: CONST_HUE },
    { type: 'util_log', message0: 'log %1', args0: [{ type: 'input_value', name: 'MSG' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: UTIL_HUE },
    { type: 'util_delay', message0: 'delay %1 ms', args0: [{ type: 'input_value', name: 'MS', check: 'Number' }], inputsInline: true, previousStatement: null, nextStatement: null, colour: UTIL_HUE }
  ]);

  var gen = javascript.javascriptGenerator;
  var Order = javascript.Order;
  gen.INFINITE_LOOP_TRAP = null;

  gen.forBlock['display_begin_scene'] = function() { return 'display.beginScene();\n'; };
  gen.forBlock['display_update']      = function() { return 'display.update();\n'; };
  gen.forBlock['display_use_title_font'] = function() { return 'display.useTitleFont();\n'; };
  gen.forBlock['display_use_body_font']  = function() { return 'display.useBodyFont();\n'; };
  gen.forBlock['display_fill_screen'] = function(block) { var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.fillScreen(' + c + ');\n'; };
  gen.forBlock['display_set_text_color'] = function(block) { var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.setTextColor(' + c + ');\n'; };
  gen.forBlock['display_set_cursor'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; return 'display.setCursor(' + x + ', ' + y + ');\n'; };
  gen.forBlock['display_print'] = function(block) { var t = gen.valueToCode(block, 'TEXT', Order.NONE) || '""'; return 'display.print(' + t + ');\n'; };
  gen.forBlock['display_set_rotation'] = function(block) { return 'display.setRotation(' + block.getFieldValue('R') + ');\n'; };
  gen.forBlock['display_width'] = function() { return ['display.width()', Order.FUNCTION_CALL]; };
  gen.forBlock['display_height'] = function() { return ['display.height()', Order.FUNCTION_CALL]; };
  gen.forBlock['display_draw_pixel'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.drawPixel(' + x + ', ' + y + ', ' + c + ');\n'; };
  gen.forBlock['display_draw_line'] = function(block) { var x1 = gen.valueToCode(block, 'X1', Order.NONE) || '0'; var y1 = gen.valueToCode(block, 'Y1', Order.NONE) || '0'; var x2 = gen.valueToCode(block, 'X2', Order.NONE) || '0'; var y2 = gen.valueToCode(block, 'Y2', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.drawLine(' + x1 + ', ' + y1 + ', ' + x2 + ', ' + y2 + ', ' + c + ');\n'; };
  gen.forBlock['display_draw_rect'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var w = gen.valueToCode(block, 'W', Order.NONE) || '0'; var h = gen.valueToCode(block, 'H', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.drawRect(' + x + ', ' + y + ', ' + w + ', ' + h + ', ' + c + ');\n'; };
  gen.forBlock['display_fill_rect'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var w = gen.valueToCode(block, 'W', Order.NONE) || '0'; var h = gen.valueToCode(block, 'H', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.fillRect(' + x + ', ' + y + ', ' + w + ', ' + h + ', ' + c + ');\n'; };
  gen.forBlock['display_draw_circle'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var r = gen.valueToCode(block, 'R', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.drawCircle(' + x + ', ' + y + ', ' + r + ', ' + c + ');\n'; };
  gen.forBlock['display_fill_circle'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var r = gen.valueToCode(block, 'R', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.fillCircle(' + x + ', ' + y + ', ' + r + ', ' + c + ');\n'; };
  gen.forBlock['display_draw_round_rect'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var w = gen.valueToCode(block, 'W', Order.NONE) || '0'; var h = gen.valueToCode(block, 'H', Order.NONE) || '0'; var cr = gen.valueToCode(block, 'CR', Order.NONE) || '5'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.drawRoundRect(' + x + ', ' + y + ', ' + w + ', ' + h + ', ' + cr + ', ' + c + ');\n'; };
  gen.forBlock['display_fill_round_rect'] = function(block) { var x = gen.valueToCode(block, 'X', Order.NONE) || '0'; var y = gen.valueToCode(block, 'Y', Order.NONE) || '0'; var w = gen.valueToCode(block, 'W', Order.NONE) || '0'; var h = gen.valueToCode(block, 'H', Order.NONE) || '0'; var cr = gen.valueToCode(block, 'CR', Order.NONE) || '5'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.fillRoundRect(' + x + ', ' + y + ', ' + w + ', ' + h + ', ' + cr + ', ' + c + ');\n'; };
  gen.forBlock['display_draw_triangle'] = function(block) { var x1 = gen.valueToCode(block, 'X1', Order.NONE) || '0'; var y1 = gen.valueToCode(block, 'Y1', Order.NONE) || '0'; var x2 = gen.valueToCode(block, 'X2', Order.NONE) || '0'; var y2 = gen.valueToCode(block, 'Y2', Order.NONE) || '0'; var x3 = gen.valueToCode(block, 'X3', Order.NONE) || '0'; var y3 = gen.valueToCode(block, 'Y3', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.drawTriangle(' + x1 + ', ' + y1 + ', ' + x2 + ', ' + y2 + ', ' + x3 + ', ' + y3 + ', ' + c + ');\n'; };
  gen.forBlock['display_fill_triangle'] = function(block) { var x1 = gen.valueToCode(block, 'X1', Order.NONE) || '0'; var y1 = gen.valueToCode(block, 'Y1', Order.NONE) || '0'; var x2 = gen.valueToCode(block, 'X2', Order.NONE) || '0'; var y2 = gen.valueToCode(block, 'Y2', Order.NONE) || '0'; var x3 = gen.valueToCode(block, 'X3', Order.NONE) || '0'; var y3 = gen.valueToCode(block, 'Y3', Order.NONE) || '0'; var c = gen.valueToCode(block, 'COLOR', Order.NONE) || 'BLACK'; return 'display.fillTriangle(' + x1 + ', ' + y1 + ', ' + x2 + ', ' + y2 + ', ' + x3 + ', ' + y3 + ', ' + c + ');\n'; };
  gen.forBlock['color_constant'] = function(block) { return [block.getFieldValue('COLOR'), Order.ATOMIC]; };
  gen.forBlock['util_log'] = function(block) { var msg = gen.valueToCode(block, 'MSG', Order.ADDITION) || '""'; return "log('' + " + msg + ");\n"; };
  gen.forBlock['util_delay'] = function(block) { var ms = gen.valueToCode(block, 'MS', Order.NONE) || '1000'; return 'delay(' + ms + ');\n'; };

  gen.forBlock['text_print'] = function(block) { var msg = gen.valueToCode(block, 'TEXT', Order.ADDITION) || '""'; return "log('' + " + msg + ");\n"; };
  gen.forBlock['text_join'] = function(block) {
    var n = block.itemCount_;
    if (n === 0) return ["''", Order.ATOMIC];
    if (n === 1) { var elem = gen.valueToCode(block, 'ADD0', Order.ADDITION) || "''"; return ["'' + " + elem, Order.ADDITION]; }
    var parts = ["''"];
    for (var i = 0; i < n; i++) { parts.push(gen.valueToCode(block, 'ADD' + i, Order.ADDITION) || "''"); }
    return [parts.join(' + '), Order.ADDITION];
  };
  gen.forBlock['text_length'] = function() { return ['0 /* .length unsupported */', Order.ATOMIC]; };

  function numShadow(val) { return { shadow: { type: 'math_number', fields: { NUM: val } } }; }
  function colorShadow() { return { shadow: { type: 'color_constant', fields: { COLOR: 'BLACK' } } }; }
  function textShadow(val) { return { shadow: { type: 'text', fields: { TEXT: val } } }; }

  var toolbox = {
    kind: 'categoryToolbox',
    contents: [
      { kind: 'category', name: 'Display', colour: '210', contents: [
        { kind: 'label', text: 'Scene' },
        { kind: 'block', type: 'display_begin_scene' },
        { kind: 'block', type: 'display_update' },
        { kind: 'block', type: 'display_fill_screen', inputs: { COLOR: colorShadow() } },
        { kind: 'sep', gap: '16' },
        { kind: 'label', text: 'Text' },
        { kind: 'block', type: 'display_use_title_font' },
        { kind: 'block', type: 'display_use_body_font' },
        { kind: 'block', type: 'display_set_text_color', inputs: { COLOR: colorShadow() } },
        { kind: 'block', type: 'display_set_cursor', inputs: { X: numShadow(10), Y: numShadow(40) } },
        { kind: 'block', type: 'display_print', inputs: { TEXT: textShadow('Hello!') } },
        { kind: 'sep', gap: '16' },
        { kind: 'label', text: 'Properties' },
        { kind: 'block', type: 'display_set_rotation' },
        { kind: 'block', type: 'display_width' },
        { kind: 'block', type: 'display_height' },
        { kind: 'sep', gap: '16' },
        { kind: 'label', text: 'Shapes' },
        { kind: 'block', type: 'display_draw_pixel', inputs: { X: numShadow(0), Y: numShadow(0), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_draw_line', inputs: { X1: numShadow(0), Y1: numShadow(0), X2: numShadow(100), Y2: numShadow(100), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_draw_rect', inputs: { X: numShadow(10), Y: numShadow(10), W: numShadow(80), H: numShadow(60), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_fill_rect', inputs: { X: numShadow(10), Y: numShadow(10), W: numShadow(80), H: numShadow(60), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_draw_circle', inputs: { X: numShadow(50), Y: numShadow(50), R: numShadow(30), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_fill_circle', inputs: { X: numShadow(50), Y: numShadow(50), R: numShadow(30), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_draw_round_rect', inputs: { X: numShadow(10), Y: numShadow(10), W: numShadow(80), H: numShadow(60), CR: numShadow(8), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_fill_round_rect', inputs: { X: numShadow(10), Y: numShadow(10), W: numShadow(80), H: numShadow(60), CR: numShadow(8), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_draw_triangle', inputs: { X1: numShadow(50), Y1: numShadow(10), X2: numShadow(10), Y2: numShadow(90), X3: numShadow(90), Y3: numShadow(90), COLOR: colorShadow() } },
        { kind: 'block', type: 'display_fill_triangle', inputs: { X1: numShadow(50), Y1: numShadow(10), X2: numShadow(10), Y2: numShadow(90), X3: numShadow(90), Y3: numShadow(90), COLOR: colorShadow() } }
      ]},
      { kind: 'category', name: 'Constants', colour: '20', contents: [{ kind: 'block', type: 'color_constant' }] },
      { kind: 'category', name: 'Utilities', colour: '160', contents: [
        { kind: 'block', type: 'util_log', inputs: { MSG: textShadow('hello') } },
        { kind: 'block', type: 'util_delay', inputs: { MS: numShadow(1000) } }
      ]},
      { kind: 'sep' },
      { kind: 'category', name: 'Logic', colour: '%{BKY_LOGIC_HUE}', contents: [
        { kind: 'block', type: 'controls_if' },
        { kind: 'block', type: 'controls_if', extraState: { hasElse: true } },
        { kind: 'block', type: 'logic_compare' },
        { kind: 'block', type: 'logic_operation' },
        { kind: 'block', type: 'logic_negate' },
        { kind: 'block', type: 'logic_boolean' }
      ]},
      { kind: 'category', name: 'Loops', colour: '%{BKY_LOOPS_HUE}', contents: [
        { kind: 'block', type: 'controls_repeat_ext', inputs: { TIMES: numShadow(10) } },
        { kind: 'block', type: 'controls_whileUntil' },
        { kind: 'block', type: 'controls_for', fields: { VAR: 'i' }, inputs: { FROM: numShadow(0), TO: numShadow(9), BY: numShadow(1) } },
        { kind: 'block', type: 'controls_flow_statements' }
      ]},
      { kind: 'category', name: 'Math', colour: '%{BKY_MATH_HUE}', contents: [
        { kind: 'block', type: 'math_number', fields: { NUM: 0 } },
        { kind: 'block', type: 'math_arithmetic', inputs: { A: numShadow(1), B: numShadow(1) } },
        { kind: 'block', type: 'math_single' },
        { kind: 'block', type: 'math_trig' },
        { kind: 'block', type: 'math_constant' },
        { kind: 'block', type: 'math_round' },
        { kind: 'block', type: 'math_modulo', inputs: { DIVIDEND: numShadow(10), DIVISOR: numShadow(3) } },
        { kind: 'block', type: 'math_constrain', inputs: { VALUE: numShadow(50), LOW: numShadow(0), HIGH: numShadow(100) } },
        { kind: 'block', type: 'math_random_int', inputs: { FROM: numShadow(1), TO: numShadow(100) } },
        { kind: 'block', type: 'math_random_float' },
        { kind: 'block', type: 'math_atan2' }
      ]},
      { kind: 'category', name: 'Text', colour: '%{BKY_TEXTS_HUE}', contents: [
        { kind: 'block', type: 'text', fields: { TEXT: 'hello' } },
        { kind: 'block', type: 'text_join' },
        { kind: 'block', type: 'text_print', inputs: { TEXT: textShadow('hello') } }
      ]},
      { kind: 'category', name: 'Variables', colour: '%{BKY_VARIABLES_HUE}', custom: 'VARIABLE' },
      { kind: 'category', name: 'Functions', colour: '%{BKY_PROCEDURES_HUE}', custom: 'PROCEDURE' }
    ]
  };

  var darkTheme = Blockly.Theme.defineTheme('kairosDark', {
    base: Blockly.Themes.Classic,
    componentStyles: { workspaceBackgroundColour: '#1e1e1e', toolboxBackgroundColour: '#333333', toolboxForegroundColour: '#fff', flyoutBackgroundColour: '#252525', flyoutForegroundColour: '#ccc', flyoutOpacity: 1, scrollbarColour: '#797979', insertionMarkerColour: '#fff', insertionMarkerOpacity: 0.3, scrollbarOpacity: 0.4, cursorColour: '#d0d0d0', blackBackground: '#333' }
  });

  var workspace = Blockly.inject('blockly-div', {
    toolbox: toolbox, theme: darkTheme,
    grid: { spacing: 20, length: 3, colour: '#333', snap: true },
    zoom: { controls: true, wheel: true, startScale: 0.9, maxScale: 3, minScale: 0.3, scaleSpeed: 1.2 },
    trashcan: true, move: { scrollbars: true, drag: true, wheel: true }, sounds: false
  });

  var generatedCodeEl = document.getElementById('generated-code');
  var lastGenerated = '';

  function normalizeElkCode(code) { return code ? code.replace(/\bvar\b/g, 'let') : ''; }
  function generateElkCode() {
    try { var code = gen.workspaceToCode(workspace); lastGenerated = normalizeElkCode(code || ''); }
    catch (e) { lastGenerated = '// Error: ' + e.message; }
    generatedCodeEl.textContent = lastGenerated || '// (empty workspace)';
    return lastGenerated;
  }
  workspace.addChangeListener(function(event) { if (!event.isUiEvent) generateElkCode(); });
  generateElkCode();

  var activeTab = 'blockly';
  var tabBtns = document.querySelectorAll('.tab-btn');
  var tabBlockly = document.getElementById('tab-blockly');
  var tabCode = document.getElementById('tab-code');
  var tabSettings = document.getElementById('tab-settings');
  var btnCopy = document.getElementById('btn-copy');
  var cm;

  function switchTab(name) {
    activeTab = name;
    tabBtns.forEach(function(b) { b.classList.toggle('active', b.getAttribute('data-tab') === name); });
    tabBlockly.classList.toggle('active', name === 'blockly');
    tabCode.classList.toggle('active', name === 'code');
    tabSettings.classList.toggle('active', name === 'settings');
    btnCopy.style.display = (name === 'blockly') ? '' : 'none';
    if (name === 'blockly') setTimeout(function() { Blockly.svgResize(workspace); }, 20);
    else if (name === 'code' && cm) setTimeout(function() { cm.refresh(); }, 20);
    else if (name === 'settings') loadSettings();
  }
  tabBtns.forEach(function(btn) { btn.addEventListener('click', function() { switchTab(this.getAttribute('data-tab')); }); });

  cm = CodeMirror.fromTextArea(document.getElementById('code'), {
    lineNumbers: true, mode: 'javascript', theme: 'monokai', lineWrapping: true, tabSize: 2, indentWithTabs: false
  });

  var output = document.getElementById('output');
  var status = document.getElementById('status');
  var dropdown = document.getElementById('scripts-dropdown');
  var nameInput = document.getElementById('save-name');

  function setStatus(msg, cls) { status.textContent = msg; status.className = 'status ' + cls; }
  function appendOutput(msg, cls) { var d = document.createElement('div'); d.className = cls || 'log'; d.textContent = msg; output.appendChild(d); output.scrollTop = output.scrollHeight; }

  async function api(method, path, body) {
    var opts = { method: method, headers: {'Content-Type': 'application/json'} };
    if (body !== undefined) opts.body = JSON.stringify(body);
    var r = await fetch(path, opts);
    return { status: r.status, data: await r.json().catch(function() { return {}; }) };
  }

  var scriptsMeta = {};
  async function loadScriptsList() {
    var res = await api('GET', '/api/scripts');
    dropdown.innerHTML = '<option value="">-- new script --</option>';
    scriptsMeta = {};
    if (Array.isArray(res.data)) {
      res.data.forEach(function(s) {
        scriptsMeta[s.name] = { type: s.type || 'js', size: s.size };
        var opt = document.createElement('option');
        opt.value = s.name;
        var icon = (s.type === 'bly') ? '\uD83E\uDDE9 ' : '\uD83D\uDCDD ';
        opt.textContent = icon + s.name + ' (' + s.size + 'B)';
        dropdown.appendChild(opt);
      });
    }
  }

  dropdown.addEventListener('change', async function() {
    var name = this.value;
    if (!name) { nameInput.value = ''; return; }
    nameInput.value = name;
    var res = await api('GET', '/api/scripts/' + encodeURIComponent(name));
    if (res.status === 200 && res.data.code !== undefined) {
      document.getElementById('save-cron').value = res.data.cron || '';
      if (typeof describeCron === 'function') document.getElementById('cron-hint').textContent = describeCron(res.data.cron || '');
      var type = res.data.type || 'js';
      if (type === 'bly') {
        try { var wsData = JSON.parse(res.data.code); workspace.clear(); Blockly.serialization.workspaces.load(wsData, workspace); switchTab('blockly'); setStatus('loaded blockly: ' + name, 'ok'); }
        catch (e) { setStatus('bad blockly data: ' + e.message, 'err'); }
      } else { cm.setValue(res.data.code); switchTab('code'); setStatus('loaded code: ' + name, 'ok'); }
    } else { setStatus('load failed', 'err'); }
  });

  document.getElementById('btn-run').addEventListener('click', async function() {
    var code;
    if (activeTab === 'blockly') { code = generateElkCode(); } else { code = cm.getValue(); }
    if (!code || !code.trim()) { setStatus('nothing to run', 'err'); return; }

    var targets = Array.from(selectedDeviceIps);
    if (targets.length === 0) targets = undefined; // default: local only

    // Check if we only target local device
    var selfDev = knownDevices.find(function(d) { return d.self; });
    var selfIp = selfDev ? selfDev.ip : null;
    var onlyLocal = targets && targets.length === 1 && targets[0] === selfIp;

    if (!targets || onlyLocal) {
      // Direct local execution
      setStatus('running locally...', 'info');
      appendOutput('> Executing (' + code.length + ' bytes) from ' + activeTab, 'log');
      var res = await api('POST', '/api/exec', { code: code });
      if (res.status === 200 && res.data.status === 'ok') { setStatus('success', 'ok'); appendOutput('OK', 'ok'); }
      else { setStatus('error', 'err'); appendOutput('ERROR: ' + (res.data.error || 'unknown'), 'err'); }
    } else {
      // Mesh execution
      setStatus('running on ' + targets.length + ' device(s)...', 'info');
      appendOutput('> Mesh exec (' + code.length + ' bytes) → ' + targets.join(', '), 'log');
      var res = await api('POST', '/api/mesh/exec', { code: code, targets: targets });
      if (res.status === 200 && res.data.results) {
        var allOk = true;
        for (var ip in res.data.results) {
          var r = res.data.results[ip];
          var name = r.name || ip;
          if (r.status === 'ok') { appendOutput(name + ': OK', 'ok'); }
          else { appendOutput(name + ': ' + r.status + (r.http_code ? ' (HTTP ' + r.http_code + ')' : ''), 'err'); allOk = false; }
        }
        setStatus(allOk ? 'all succeeded' : 'some failed', allOk ? 'ok' : 'err');
      } else {
        setStatus('mesh exec failed', 'err');
        appendOutput('ERROR: ' + (res.data.error || 'unknown'), 'err');
      }
    }
  });

  document.getElementById('btn-save').addEventListener('click', async function() {
    var name = nameInput.value.trim();
    if (!name) { setStatus('enter a name', 'err'); return; }
    if (!/^[a-zA-Z0-9_-]+$/.test(name)) { setStatus('name: alphanumeric/-/_', 'err'); return; }
    var code, type;
    if (activeTab === 'blockly') { var wsState = Blockly.serialization.workspaces.save(workspace); code = JSON.stringify(wsState); type = 'bly'; }
    else { code = cm.getValue(); type = 'js'; }
    var cronStr = document.getElementById('save-cron').value.trim();

    var targets = Array.from(selectedDeviceIps);
    if (targets.length === 0) targets = undefined; // default: local only

    var selfDev = knownDevices.find(function(d) { return d.self; });
    var selfIp = selfDev ? selfDev.ip : null;
    var onlyLocal = targets && targets.length === 1 && targets[0] === selfIp;

    if (!targets || onlyLocal) {
      document.getElementById('btn-save').textContent = 'Saving...';
      var res = await api('POST', '/api/scripts', { name: name, code: code, type: type, cron: cronStr });
      document.getElementById('btn-save').textContent = 'Save';
      if (res.status === 200) { setStatus('saved: ' + name, 'ok'); await loadScriptsList(); dropdown.value = name; }
      else { setStatus('save failed: ' + (res.data.error || ''), 'err'); }
    } else {
      setStatus('saving to ' + targets.length + ' device(s)...', 'info');
      document.getElementById('btn-save').textContent = 'Saving...';
      var payload = { name: name, code: code, type: type, cron: cronStr, targets: targets };
      var res = await api('POST', '/api/mesh/scripts', payload);
      document.getElementById('btn-save').textContent = 'Save';
      if (res.status === 200 && res.data.results) {
        var allOk = true;
        for (var ip in res.data.results) {
          var r = res.data.results[ip];
          var iconEl = document.getElementById('save-icon-' + ip.replace(/\./g, '-'));
          if (r.status === 'ok') { 
            if (iconEl) { iconEl.innerHTML = '\u2705'; iconEl.style.display = 'inline'; }
          } else { 
            if (iconEl) { iconEl.innerHTML = '\u274c'; iconEl.style.display = 'inline'; }
            allOk = false; 
          }
        }
        setStatus(allOk ? 'saved to all' : 'some saves failed', allOk ? 'ok' : 'err');
        await loadScriptsList(); dropdown.value = name;
      } else {
        setStatus('mesh save failed', 'err');
      }
    }
  });

  document.getElementById('btn-delete').addEventListener('click', async function() {
    var name = nameInput.value.trim();
    if (!name) { setStatus('no script selected', 'err'); return; }
    if (!confirm('Delete "' + name + '"?')) return;
    var res = await api('DELETE', '/api/scripts/' + encodeURIComponent(name));
    if (res.status === 200) { setStatus('deleted: ' + name, 'ok'); nameInput.value = ''; cm.setValue(''); workspace.clear(); await loadScriptsList(); }
    else { setStatus('delete failed', 'err'); }
  });

  btnCopy.addEventListener('click', function() { var code = generateElkCode(); cm.setValue(code); switchTab('code'); setStatus('copied to editor', 'ok'); });
  document.getElementById('btn-copy-code').addEventListener('click', function() { if (navigator.clipboard) { navigator.clipboard.writeText(lastGenerated); setStatus('copied!', 'ok'); } });
  document.addEventListener('keydown', function(ev) { if (ev.keyCode === 13 && (ev.metaKey || ev.ctrlKey)) { ev.preventDefault(); document.getElementById('btn-run').click(); } });

  loadScriptsList();
  setStatus('ready', 'info');

  // ── Cron helper: show human-readable description ──────────────────────
  var cronInput = document.getElementById('save-cron');
  var cronHint = document.getElementById('cron-hint');
  function describeCron(expr) {
    if (!expr) return '';
    var p = expr.trim().split(/\s+/);
    if (p.length < 5) return '\u26a0 need 5 fields: MIN HR DOM MON DOW';
    if (p.length > 5) return '\u26a0 too many fields (expected 5)';
    var mn = p[0], hr = p[1], dom = p[2], mon = p[3], dow = p[4];
    // Validate each field
    function validField(f, lo, hi) {
      var parts = f.split(',');
      for (var i = 0; i < parts.length; i++) {
        var t = parts[i];
        var step = t.split('/');
        if (step.length > 2) return false;
        if (step.length === 2 && (isNaN(step[1]) || +step[1] < 1)) return false;
        var base = step[0];
        if (base === '*') continue;
        var range = base.split('-');
        if (range.length > 2) return false;
        for (var j = 0; j < range.length; j++) {
          if (isNaN(range[j])) return false;
          var v = +range[j];
          if (v < lo || v > hi) return false;
        }
      }
      return true;
    }
    if (!validField(mn, 0, 59)) return '\u26a0 bad minute field';
    if (!validField(hr, 0, 23)) return '\u26a0 bad hour field';
    if (!validField(dom, 1, 31)) return '\u26a0 bad day-of-month';
    if (!validField(mon, 1, 12)) return '\u26a0 bad month field';
    if (!validField(dow, 0, 6)) return '\u26a0 bad day-of-week (0=Sun)';
    // Build readable description
    var parts = [];
    if (mn === '*') parts.push('every min');
    else if (mn.indexOf('/') > 0) parts.push('every ' + mn.split('/')[1] + ' min');
    else parts.push('at :' + mn.padStart(2, '0'));
    if (hr === '*') parts.push('every hr');
    else if (hr.indexOf('/') > 0) parts.push('every ' + hr.split('/')[1] + 'h');
    else parts.push(hr + ':00');
    if (dom !== '*') parts.push('day ' + dom);
    var months = ['','Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
    if (mon !== '*') parts.push(months[parseInt(mon)] || mon);
    var days = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];
    if (dow !== '*') {
      var dowParts = dow.split(',').map(function(d) { return days[parseInt(d)] || d; });
      parts.push(dowParts.join(','));
    }
    if (mn !== '*' && hr !== '*' && dom === '*' && mon === '*' && dow === '*')
      return '\u2705 Daily at ' + hr.padStart(2,'0') + ':' + mn.padStart(2,'0');
    if (mn.indexOf('/') >= 0 && hr === '*' && dom === '*' && mon === '*' && dow === '*')
      return '\u2705 Every ' + mn.split('/')[1] + ' min';
    return '\u2705 ' + parts.join(', ');
  }
  cronInput.addEventListener('input', function() { cronHint.textContent = describeCron(this.value); });
  cronInput.addEventListener('focus', function() { if (!this.value) this.placeholder = 'MIN HR DOM MON DOW'; });
  cronInput.addEventListener('blur', function() { this.placeholder = 'cron (min hr * * *)'; });

  // ── Mesh devices panel ────────────────────────────────────────────────
  var devicesList = document.getElementById('devices-list');
  var knownDevices = [];
  var selectedDeviceIps = new Set();

  async function loadMeshPeers() {
    var res = await api('GET', '/api/mesh/peers');
    if (res.status === 200 && Array.isArray(res.data)) {
      var now = new Date();
      var localEpoch = Math.floor(now.getTime() / 1000);
      knownDevices = res.data;
      // Auto-select self on first load
      if (selectedDeviceIps.size === 0) {
        knownDevices.forEach(function(d) { if (d.self) selectedDeviceIps.add(d.ip); });
      }
      var selfDev = knownDevices.find(function(d) { return d.self === true; });
      if (selfDev && selfDev.time) {
        selfDev.epoch = localEpoch;
        selfDev.time = now.getHours().toString().padStart(2,'0') + ':' + now.getMinutes().toString().padStart(2,'0') + ':' + now.getSeconds().toString().padStart(2,'0');
      }
      renderDevices();
      // Fetch time from each remote peer in parallel
      var remotes = knownDevices.filter(function(d) { return !d.self && d.ip; });
      if (remotes.length > 0) {
        var fetches = remotes.map(function(d) {
          return fetch('http://' + d.ip + '/api/status', { signal: AbortSignal.timeout(3000) })
            .then(function(r) { return r.json(); })
            .then(function(data) { d.time = data.time || null; d.epoch = data.epoch || null; })
            .catch(function() { d.time = null; d.epoch = null; });
        });
        Promise.all(fetches).then(renderDevices);
      }
    }
  }

  function renderDevices() {
    devicesList.innerHTML = '';
    if (knownDevices.length === 0) {
      devicesList.innerHTML = '<div style="color:#666;font-size:11px;">No devices found</div>';
      return;
    }
    knownDevices.forEach(function(d) {
      var div = document.createElement('div');
      div.className = 'device-item';
      var cb = document.createElement('input');
      cb.type = 'checkbox';
      cb.checked = selectedDeviceIps.has(d.ip);
      cb.addEventListener('change', function() {
        if (this.checked) selectedDeviceIps.add(d.ip);
        else selectedDeviceIps.delete(d.ip);
        updateRemoteApi();
      });
      var info = document.createElement('div');
      var timeStr = d.time ? ' <span style="color:#aaa" class="clock-tick" data-epoch="' + (d.epoch || '') + '">\u23f0 ' + d.time + '</span>' : '';
      var saveIcon = '<span id="save-icon-' + d.ip.replace(/\./g, '-') + '" style="display:none; margin-left:6px;"></span>';
      info.innerHTML = '<div class="dev-name">' + d.name + '</div>'
        + '<div class="dev-ip">' + d.ip + timeStr + saveIcon + '</div>'
        + '<div class="' + (d.self ? 'dev-self' : 'dev-remote') + '">'
        + (d.self ? '(this device)' : 'peer') + '</div>';
      div.appendChild(cb);
      div.appendChild(info);
      devicesList.appendChild(div);
    });
  }

  setInterval(function() {
    document.querySelectorAll('.clock-tick').forEach(function(el) {
      var epochStr = el.getAttribute('data-epoch');
      if (epochStr && !isNaN(epochStr)) {
        var epoch = parseInt(epochStr, 10) + 1;
        el.setAttribute('data-epoch', epoch);
        var d = new Date(epoch * 1000);
        var hh = d.getHours().toString().padStart(2, '0');
        var mm = d.getMinutes().toString().padStart(2, '0');
        var ss = d.getSeconds().toString().padStart(2, '0');
        el.innerHTML = '\u23f0 ' + hh + ':' + mm + ':' + ss;
      }
    });
  }, 1000);

  async function updateRemoteApi() {
    var panel = document.getElementById('info-panel');
    // If exactly one remote device selected, fetch its API
    var selected = Array.from(selectedDeviceIps);
    var remoteIps = selected.filter(function(ip) {
      var d = knownDevices.find(function(dev) { return dev.ip === ip; });
      return d && !d.self;
    });

    if (remoteIps.length === 1) {
      var ip = remoteIps[0];
      var dev = knownDevices.find(function(d) { return d.ip === ip; });
      panel.innerHTML = '<h4>API: ' + (dev ? dev.name : ip) + '</h4><div style="color:#888">Loading...</div>';
      var res = await api('GET', '/api/mesh/api?ip=' + encodeURIComponent(ip));
      if (res.status === 200 && res.data.objects) {
        var html = '<h4>API: ' + (dev ? dev.name : ip) + '</h4>';
        for (var objName in res.data.objects) {
          html += '<h4>' + objName + '</h4><ul>';
          var obj = res.data.objects[objName];
          if (obj.methods) {
            for (var m in obj.methods) {
              var meth = obj.methods[m];
              var sig = objName + '.' + m + '(' + (meth.args||[]).join(', ') + ')';
              var desc = meth.desc ? ' — ' + meth.desc : '';
              var ret = meth.ret ? ' → ' + meth.ret : '';
              html += '<li><code>' + sig + '</code>' + ret + desc + '</li>';
            }
          }
          if (obj.constants) {
            html += '<li><code>' + obj.constants.join(', ') + '</code></li>';
          }
          html += '</ul>';
        }
        if (res.data.globals) {
          html += '<h4>Globals</h4><ul>';
          for (var g in res.data.globals) {
            var gl = res.data.globals[g];
            if (gl.args) html += '<li><code>' + g + '(' + gl.args.join(', ') + ')</code>' + (gl.desc ? ' — ' + gl.desc : '') + '</li>';
            else if (gl.value !== undefined) html += '<li><code>' + g + '</code> = ' + gl.value + '</li>';
          }
          html += '</ul>';
        }
        panel.innerHTML = html;
      } else {
        panel.innerHTML = '<h4>API: ' + (dev ? dev.name : ip) + '</h4><div style="color:#f66">Failed to fetch API</div>';
      }
    }
    // If no remote selected or multiple, show local API reference (default)
    else {
      resetLocalApiPanel();
    }
  }

  var localApiHtml = document.getElementById('info-panel').innerHTML;
  function resetLocalApiPanel() {
    document.getElementById('info-panel').innerHTML = localApiHtml;
  }

  document.getElementById('btn-refresh-peers').addEventListener('click', loadMeshPeers);

  // Auto-refresh peers every 8s
  setInterval(loadMeshPeers, 8000);
  loadMeshPeers();

  // ── Settings tab ──────────────────────────────────────────────────────
  var cfgStatus = document.getElementById('cfg-status');
  function setCfgStatus(msg, cls) { cfgStatus.textContent = msg; cfgStatus.className = 'status ' + cls; }

  async function loadSettings() {
    var res = await api('GET', '/api/config');
    if (res.status === 200) {
      document.getElementById('cfg-device-name').value = res.data.device_name || '';
      document.getElementById('cfg-ws-enabled').checked = !!res.data.ws_enabled;
      document.getElementById('cfg-ws-host').value = res.data.ws_host || '';
      document.getElementById('cfg-ws-port').value = res.data.ws_port || 443;
      document.getElementById('cfg-ws-path').value = res.data.ws_path || '/kairos/{device_id}';
      document.getElementById('cfg-ws-ssl').checked = res.data.ws_ssl !== false;
      document.getElementById('cfg-ntp-server').value = res.data.ntp_server || '';
      document.getElementById('cfg-tz-info').value = res.data.tz_info || '';

    }
    var st = await api('GET', '/api/status');
    if (st.status === 200) {
      document.getElementById('st-ip').textContent = 'IP: ' + (st.data.ip || '?');
      document.getElementById('st-heap').textContent = 'Heap: ' + (st.data.heap_free || '?') + ' bytes';
      document.getElementById('st-uptime').textContent = 'Uptime: ' + Math.round((st.data.uptime_ms || 0) / 1000) + 's';
      document.getElementById('st-ws').textContent = 'WS Client: ' + (st.data.ws_connected ? 'connected' : 'disconnected');

      document.getElementById('st-rssi').textContent = 'WiFi RSSI: ' + (st.data.wifi_rssi || '?') + ' dBm';
      document.getElementById('st-time').textContent = 'Device Time: ' + (st.data.time ? st.data.date + ' ' + st.data.time : 'not synced');
    }
  }

  document.getElementById('btn-cfg-save').addEventListener('click', async function() {
    var cfg = {
      device_name: document.getElementById('cfg-device-name').value,
      ws_enabled: document.getElementById('cfg-ws-enabled').checked,
      ws_host: document.getElementById('cfg-ws-host').value,
      ws_port: parseInt(document.getElementById('cfg-ws-port').value) || 443,
      ws_path: document.getElementById('cfg-ws-path').value,
      ws_ssl: document.getElementById('cfg-ws-ssl').checked,
      ntp_server: document.getElementById('cfg-ntp-server').value,
      tz_info: document.getElementById('cfg-tz-info').value,
      mqtt_enabled: document.getElementById('cfg-mqtt-enabled').checked,
      mqtt_broker: document.getElementById('cfg-mqtt-broker').value,
      mqtt_port: parseInt(document.getElementById('cfg-mqtt-port').value) || 1883,
      mqtt_user: document.getElementById('cfg-mqtt-user').value,
      mqtt_pass: document.getElementById('cfg-mqtt-pass').value,
      mqtt_topic_prefix: document.getElementById('cfg-mqtt-prefix').value || 'kairos'
    };
    var res = await api('POST', '/api/config', cfg);
    if (res.status === 200) { setCfgStatus('Saved! Restart device to apply WS changes.', 'ok'); }
    else { setCfgStatus('Save failed', 'err'); }
  });

  // ── Draggable Splitters ───────────────────────────────────────────────
  (function() {
    var dragging = null;
    var vsplitB = document.getElementById('vsplit-blockly');
    var codePreview = document.getElementById('code-preview-panel');
    var blocklyLayout = vsplitB ? vsplitB.parentElement : null;
    var vsplitC = document.getElementById('vsplit-code');
    var infoPanel = document.getElementById('info-panel');
    var codeLayout = vsplitC ? vsplitC.parentElement : null;
    var hsplit = document.getElementById('hsplit');
    var outputEl = document.getElementById('output');

    if (vsplitB) vsplitB.addEventListener('mousedown', function(e) { e.preventDefault(); dragging = 'vb'; });
    if (vsplitC) vsplitC.addEventListener('mousedown', function(e) { e.preventDefault(); dragging = 'vc'; });
    if (hsplit) hsplit.addEventListener('mousedown', function(e) { e.preventDefault(); dragging = 'h'; });

    document.addEventListener('mousemove', function(e) {
      if (!dragging) return;
      e.preventDefault();
      if (dragging === 'vb' && blocklyLayout) { var rect = blocklyLayout.getBoundingClientRect(); var newW = rect.right - e.clientX - 3; if (newW < 120) newW = 120; if (newW > rect.width - 200) newW = rect.width - 200; codePreview.style.width = newW + 'px'; Blockly.svgResize(workspace); }
      else if (dragging === 'vc' && codeLayout) { var rect = codeLayout.getBoundingClientRect(); var newW = rect.right - e.clientX - 3; if (newW < 150) newW = 150; if (newW > rect.width - 200) newW = rect.width - 200; infoPanel.style.width = newW + 'px'; if (cm) cm.refresh(); }
      else if (dragging === 'h') { var bh = document.body.getBoundingClientRect().height; var newH = bh - e.clientY - 3; if (newH < 40) newH = 40; if (newH > bh - 200) newH = bh - 200; outputEl.style.height = newH + 'px'; if (activeTab === 'blockly') Blockly.svgResize(workspace); else if (cm) cm.refresh(); }
    });
    document.addEventListener('mouseup', function() { dragging = null; });
  })();

  window.addEventListener('resize', function() { if (activeTab === 'blockly') Blockly.svgResize(workspace); else if (cm) cm.refresh(); });
})();
  </script>
</body>
</html>)rawhtml";
