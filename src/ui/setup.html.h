#pragma once
static const char SETUP_HTML[] PROGMEM = R"rawhtml(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Kairos Ink Setup</title>
  <link rel="icon" href="/favicon.ico" sizes="any">
  <style>
    :root {
      --paper: #f4f0e8;
      --ink: #1f1a17;
      --muted: #6a625c;
      --line: #d6c9b7;
      --panel: #fffaf2;
      --accent: #1d5c4b;
      --accent-2: #d88a2d;
      --danger: #9d3d2f;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Avenir Next", "Segoe UI", sans-serif;
      color: var(--ink);
      background:
        radial-gradient(circle at top left, rgba(216,138,45,0.16), transparent 26%),
        linear-gradient(180deg, #fbf7f1, #efe6d8);
      min-height: 100vh;
    }
    .wrap {
      max-width: 760px;
      margin: 0 auto;
      padding: 32px 18px 40px;
    }
    .hero {
      padding: 22px 24px;
      border: 1px solid var(--line);
      background: rgba(255,250,242,0.86);
      border-radius: 22px;
      box-shadow: 0 18px 40px rgba(54, 40, 25, 0.08);
      backdrop-filter: blur(8px);
    }
    h1 {
      margin: 0 0 10px;
      font-size: clamp(32px, 5vw, 52px);
      line-height: 0.95;
      letter-spacing: -0.04em;
    }
    .subtitle, .hint, .meta {
      color: var(--muted);
      line-height: 1.5;
    }
    .network {
      display: inline-block;
      margin-top: 12px;
      padding: 8px 12px;
      border-radius: 999px;
      background: #efe3cf;
      color: var(--ink);
      font-weight: 700;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      font-size: 12px;
    }
    form {
      margin-top: 22px;
      display: grid;
      gap: 14px;
    }
    .grid {
      display: grid;
      gap: 14px;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    }
    label {
      display: grid;
      gap: 7px;
      font-size: 14px;
      font-weight: 600;
    }
    input {
      width: 100%;
      padding: 12px 13px;
      border-radius: 12px;
      border: 1px solid var(--line);
      background: var(--panel);
      color: var(--ink);
      font: inherit;
    }
    input:focus {
      outline: 2px solid rgba(29,92,75,0.2);
      border-color: var(--accent);
    }
    button {
      border: 0;
      border-radius: 14px;
      padding: 14px 18px;
      font: inherit;
      font-weight: 700;
      color: #fff;
      background: linear-gradient(135deg, var(--accent), #2f7d67);
      cursor: pointer;
    }
    button:disabled {
      opacity: 0.6;
      cursor: wait;
    }
    .status {
      min-height: 24px;
      font-weight: 600;
    }
    .status.ok { color: var(--accent); }
    .status.err { color: var(--danger); }
    .card {
      margin-top: 18px;
      padding: 18px;
      border-radius: 18px;
      border: 1px solid var(--line);
      background: rgba(255,255,255,0.64);
    }
    @media (max-width: 640px) {
      .wrap { padding-top: 18px; }
      .hero { padding: 18px; border-radius: 18px; }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <section class="hero">
      <div class="meta">Kairos setup mode</div>
      <h1>Connect this device to your WiFi.</h1>
      <div class="subtitle">Join the temporary device network, enter the destination WiFi details below, save, and wait for the device to restart itself.</div>
      <div class="network">kairos-ink</div>

      <form id="setup-form">
        <div class="grid">
          <label>
            Device name
            <input id="device_name" name="device_name" maxlength="31" placeholder="kairos-001">
          </label>
          <label>
            WiFi SSID
            <input id="wifi_ssid" name="wifi_ssid" maxlength="64" required placeholder="Your network name">
          </label>
          <label>
            WiFi password
            <input id="wifi_pass" name="wifi_pass" type="password" maxlength="64" placeholder="Leave blank for open networks">
          </label>
          <label>
            NTP server
            <input id="ntp_server" name="ntp_server" maxlength="64" placeholder="pool.ntp.org">
          </label>
          <label style="grid-column: 1 / -1;">
            Time zone info
            <input id="tz_info" name="tz_info" maxlength="96" placeholder="CET-1CEST,M3.5.0,M10.5.0/3">
          </label>
        </div>
        <button id="save-btn" type="submit">Save and restart</button>
        <div id="status" class="status"></div>
      </form>
    </section>

    <section class="card hint">
      After restart, reconnect your phone or laptop to the same WiFi network as the device and open the address shown on the e-paper display.
    </section>
  </div>

  <script>
    (function() {
      var form = document.getElementById('setup-form');
      var saveBtn = document.getElementById('save-btn');
      var statusEl = document.getElementById('status');

      function setStatus(msg, cls) {
        statusEl.textContent = msg;
        statusEl.className = 'status ' + (cls || '');
      }

      fetch('/api/config')
        .then(function(res) { return res.json(); })
        .then(function(cfg) {
          document.getElementById('device_name').value = cfg.device_name || '';
          document.getElementById('wifi_ssid').value = cfg.wifi_ssid || '';
          document.getElementById('ntp_server').value = cfg.ntp_server || '';
          document.getElementById('tz_info').value = cfg.tz_info || '';
        })
        .catch(function() {
          setStatus('Could not load current settings.', 'err');
        });

      form.addEventListener('submit', function(ev) {
        ev.preventDefault();
        saveBtn.disabled = true;
        setStatus('Saving settings and restarting...', '');

        var payload = {
          device_name: document.getElementById('device_name').value.trim(),
          wifi_ssid: document.getElementById('wifi_ssid').value.trim(),
          wifi_pass: document.getElementById('wifi_pass').value,
          ntp_server: document.getElementById('ntp_server').value.trim(),
          tz_info: document.getElementById('tz_info').value.trim()
        };

        fetch('/api/setup', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        }).then(function(res) {
          return res.json().catch(function() { return {}; }).then(function(data) {
            return { status: res.status, data: data };
          });
        }).then(function(res) {
          if (res.status === 200) {
            setStatus('Saved. The device is restarting now.', 'ok');
            return;
          }
          saveBtn.disabled = false;
          setStatus(res.data.error || 'Save failed.', 'err');
        }).catch(function() {
          saveBtn.disabled = false;
          setStatus('Network error while saving settings.', 'err');
        });
      });
    })();
  </script>
</body>
</html>
)rawhtml";