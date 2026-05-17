#pragma once
static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>Kairos Editor</title>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <link rel="icon" href="/favicon.ico" sizes="any"/>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.16/codemirror.min.css"/>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.16/theme/monokai.min.css"/>
  <link rel="stylesheet" href="/ui/style.css"/>
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
    <button class="tab-btn" data-tab="blockly">Blockly</button>
    <button class="tab-btn active" data-tab="code">Code Editor</button>
    <button class="tab-btn" data-tab="settings">Settings</button>
  </div>
  <div id="tab-blockly" class="tab-content">
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
  <div id="tab-code" class="tab-content active">
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
        <h4>MQTT</h4>
        <ul>
          <li><code>mqtt.publish(topic, payload)</code></li>
          <li><code>mqtt.subscribe(topic)</code></li>
          <li><code>mqtt.receive()</code> — {topic, payload} or ""</li>
          <li><code>mqtt.connected()</code> — 1 or 0</li>
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
      <h3>MQTT Client</h3>
      <div class="field checkbox">
        <input type="checkbox" id="cfg-mqtt-enabled"/>
        <label for="cfg-mqtt-enabled">Enable MQTT client</label>
      </div>
      <div class="field">
        <label>MQTT Broker</label>
        <input type="text" id="cfg-mqtt-broker" placeholder="mqtt.example.com"/>
      </div>
      <div class="field">
        <label>MQTT Port</label>
        <input type="number" id="cfg-mqtt-port" value="1883"/>
      </div>
      <div class="field">
        <label>MQTT Username</label>
        <input type="text" id="cfg-mqtt-user" placeholder="(optional)"/>
      </div>
      <div class="field">
        <label>MQTT Password</label>
        <input type="password" id="cfg-mqtt-pass" placeholder="(optional)"/>
      </div>
      <div class="field">
        <label>MQTT Topic Prefix</label>
        <input type="text" id="cfg-mqtt-prefix" placeholder="kairos"/>
      </div>
      <button class="primary" id="btn-cfg-save">Save &amp; Apply</button>
      <span id="cfg-status" class="status info" style="margin-left:12px;"></span>
      <div class="status-info" id="device-status">
        <div><strong>Device Status</strong></div>
        <div id="st-ip">IP: —</div>
        <div id="st-heap">Heap: —</div>
        <div id="st-uptime">Uptime: —</div>
        <div id="st-ws">WS Client: —</div>
        <div id="st-mqtt">MQTT: —</div>
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
  <script src="/ui/app.js"></script>
</body>
</html>
)rawhtml";
