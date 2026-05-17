#pragma once
static const char STYLE_CSS[] PROGMEM = R"rawcss(
* { box-sizing: border-box; margin: 0; padding: 0; }
html, body { height: 100%; font-family: -apple-system, BlinkMacSystemFont, sans-serif; background: #1e1e1e; color: #ccc; display: flex; flex-direction: column; overflow: hidden; }

/* ── Toolbar ─────────────────────────────────────────────────────────── */
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
#save-name { width: 140px; }

/* ── Status badges ───────────────────────────────────────────────────── */
.status.ok { background: #1a4a1a; color: #6f6; }
.status.err { background: #4a1a1a; color: #f66; }
.status.info { background: #1a3a4a; color: #6cf; }

/* ── Tab bar ─────────────────────────────────────────────────────────── */
.tab-bar { display: flex; background: #2d2d2d; border-bottom: 2px solid #444; flex-shrink: 0; }
.tab-bar button { padding: 7px 22px; background: transparent; border: none; border-bottom: 2px solid transparent; color: #888; font-size: 13px; font-weight: 600; cursor: pointer; margin-bottom: -2px; }
.tab-bar button:hover { color: #ccc; background: #333; }
.tab-bar button.active { color: #eee; border-bottom-color: #6cf; background: #1e1e1e; }
.tab-content { display: none; flex: 1; overflow: hidden; }
.tab-content.active { display: flex; }

/* ── Blockly tab ─────────────────────────────────────────────────────── */
.blockly-layout { display: flex; flex: 1; overflow: hidden; }
#blockly-area { flex: 1; position: relative; min-width: 200px; }
#blockly-div { position: absolute; top: 0; left: 0; right: 0; bottom: 0; }
.code-preview { width: 300px; min-width: 120px; display: flex; flex-direction: column; background: #252525; border-left: 1px solid #444; }
.code-preview-hdr { display: flex; align-items: center; justify-content: space-between; padding: 6px 12px; background: #2d2d2d; border-bottom: 1px solid #444; flex-shrink: 0; }
.code-preview-hdr h4 { color: #eee; font-size: 13px; margin: 0; }
.code-preview-hdr button { font-size: 11px; padding: 2px 8px; border-radius: 3px; border: 1px solid #555; background: #3a3a3a; color: #ccc; cursor: pointer; }
.code-preview-hdr button:hover { background: #505050; }
.code-preview pre { flex: 1; overflow: auto; padding: 10px 12px; margin: 0; font-family: 'SF Mono', Monaco, Menlo, monospace; font-size: 12px; color: #8cf; white-space: pre-wrap; line-height: 1.5; }

/* ── Code editor tab ─────────────────────────────────────────────────── */
.code-layout { display: flex; flex: 1; overflow: hidden; }
.left-pane { display: flex; flex-direction: column; flex: 1; min-width: 200px; overflow: hidden; }
.editor-wrap { flex: 1; overflow: hidden; }
.CodeMirror { height: 100%; font-size: 13px; }

/* ── Info / API panel ────────────────────────────────────────────────── */
.info-panel { width: 400px; min-width: 150px; overflow-y: auto; padding: 12px 14px; background: #252525; font-size: 12px; color: #aaa; line-height: 1.6; }
.info-panel h4 { color: #eee; margin: 12px 0 6px 0; font-size: 13px; }
.info-panel h4:first-child { margin-top: 0; }
.info-panel ul { margin: 0 0 0 16px; padding: 0; }
.info-panel li { margin: 2px 0; }
.info-panel code { color: #8cf; font-size: 11px; }

/* ── Devices panel ───────────────────────────────────────────────────── */
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

/* ── Splitters ───────────────────────────────────────────────────────── */
.vsplit { flex: 0 0 6px; background: #444; cursor: col-resize; user-select: none; -webkit-user-select: none; }
.vsplit:hover { background: #666; }
.hsplit { flex: 0 0 6px; background: #444; cursor: row-resize; user-select: none; -webkit-user-select: none; }
.hsplit:hover { background: #666; }

/* ── Output console ──────────────────────────────────────────────────── */
.output { height: 120px; min-height: 40px; overflow-y: auto; background: #111; padding: 8px; font-family: monospace; font-size: 12px; white-space: pre-wrap; flex-shrink: 0; border-top: 1px solid #444; }
.output .err { color: #f66; }
.output .ok { color: #6f6; }
.output .log { color: #ccc; }

/* ── Settings tab ────────────────────────────────────────────────────── */
.settings-layout { flex: 1; overflow-y: auto; padding: 24px; max-width: 600px; }
.settings-layout h3 { color: #eee; margin: 0 0 16px 0; }
.settings-layout .field { margin-bottom: 14px; }
.settings-layout label { display: block; font-size: 12px; color: #aaa; margin-bottom: 4px; }
.settings-layout input[type=text],
.settings-layout input[type=number],
.settings-layout input[type=password] {
  width: 100%; padding: 6px 10px; border-radius: 4px; border: 1px solid #555; background: #3a3a3a; color: #eee; font-size: 13px;
}
.settings-layout .checkbox { display: flex; align-items: center; gap: 8px; }
.settings-layout button.primary { margin-top: 12px; }
.settings-layout .status-info { margin-top: 20px; padding: 12px; background: #252525; border-radius: 6px; font-size: 12px; }
.settings-layout .status-info div { margin: 4px 0; }

/* ── Responsive ──────────────────────────────────────────────────────── */
@media (max-width: 700px) {
  .code-layout { flex-direction: column; }
  .blockly-layout { flex-direction: column; }
  .info-panel { width: auto !important; max-height: 200px; }
  .code-preview { width: auto !important; max-height: 200px; }
  .vsplit { display: none; }
  .toolbar { gap: 4px; }
  #save-name { width: 100px; }
}
)rawcss";
