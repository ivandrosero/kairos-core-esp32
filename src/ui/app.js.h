#pragma once
static const char APP_JS[] PROGMEM = R"rawjs(
(function() {
  'use strict';

  // ════════════════════════════════════════════════════════════════════════
  //  §1  Blockly block definitions & code generators
  // ════════════════════════════════════════════════════════════════════════

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

  // ════════════════════════════════════════════════════════════════════════
  //  §2  Blockly toolbox
  // ════════════════════════════════════════════════════════════════════════

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

  // ════════════════════════════════════════════════════════════════════════
  //  §3  Blockly workspace
  // ════════════════════════════════════════════════════════════════════════

  var darkTheme = Blockly.Theme.defineTheme('kairosDark', {
    base: Blockly.Themes.Classic,
    componentStyles: {
      workspaceBackgroundColour: '#1e1e1e', toolboxBackgroundColour: '#333333',
      toolboxForegroundColour: '#fff', flyoutBackgroundColour: '#252525',
      flyoutForegroundColour: '#ccc', flyoutOpacity: 1, scrollbarColour: '#797979',
      insertionMarkerColour: '#fff', insertionMarkerOpacity: 0.3,
      scrollbarOpacity: 0.4, cursorColour: '#d0d0d0', blackBackground: '#333'
    }
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

  // ════════════════════════════════════════════════════════════════════════
  //  §4  Tab switching & CodeMirror
  // ════════════════════════════════════════════════════════════════════════

  var activeTab = 'code';
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
  // Default tab is 'code' — make sure CodeMirror paints after layout settles
  setTimeout(function() { cm.refresh(); }, 50);

  // ════════════════════════════════════════════════════════════════════════
  //  §5  Helpers
  // ════════════════════════════════════════════════════════════════════════

  var output = document.getElementById('output');
  var status = document.getElementById('status');
  var dropdown = document.getElementById('scripts-dropdown');
  var nameInput = document.getElementById('save-name');

  function setStatus(msg, cls) { status.textContent = msg; status.className = 'status ' + cls; }
  function appendOutput(msg, cls) {
    var d = document.createElement('div');
    d.className = cls || 'log';
    d.textContent = msg;
    output.appendChild(d);
    output.scrollTop = output.scrollHeight;
  }

  function api(method, path, body) {
    var opts = { method: method, headers: {'Content-Type': 'application/json'} };
    if (body !== undefined) opts.body = JSON.stringify(body);
    return fetch(path, opts).then(function(r) {
      return r.json().catch(function() { return {}; }).then(function(data) {
        return { status: r.status, data: data };
      });
    });
  }

  // ════════════════════════════════════════════════════════════════════════
  //  §6  Scripts list (load / save / delete / run)
  // ════════════════════════════════════════════════════════════════════════

  var scriptsMeta = {};

  function loadScriptsList() {
    return api('GET', '/api/scripts').then(function(res) {
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
    });
  }

  dropdown.addEventListener('change', function() {
    var name = this.value;
    if (!name) { nameInput.value = ''; return; }
    nameInput.value = name;
    api('GET', '/api/scripts/' + encodeURIComponent(name)).then(function(res) {
      if (res.status === 200 && res.data.code !== undefined) {
        document.getElementById('save-cron').value = res.data.cron || '';
        cronHint.textContent = describeCron(res.data.cron || '');
        var type = res.data.type || 'js';
        if (type === 'bly') {
          try { var wsData = JSON.parse(res.data.code); workspace.clear(); Blockly.serialization.workspaces.load(wsData, workspace); switchTab('blockly'); setStatus('loaded blockly: ' + name, 'ok'); }
          catch (e) { setStatus('bad blockly data: ' + e.message, 'err'); }
        } else { cm.setValue(res.data.code); switchTab('code'); setStatus('loaded code: ' + name, 'ok'); }
      } else { setStatus('load failed', 'err'); }
    });
  });

  // ── Run button ──
  document.getElementById('btn-run').addEventListener('click', function() {
    var code;
    if (activeTab === 'blockly') { code = generateElkCode(); } else { code = cm.getValue(); }
    if (!code || !code.trim()) { setStatus('nothing to run', 'err'); return; }

    var targets = Array.from(selectedDeviceIps);
    var selfDev = knownDevices.find(function(d) { return d.self; });
    var selfIp = selfDev ? selfDev.ip : null;
    var onlyLocal = targets.length === 0 || (targets.length === 1 && targets[0] === selfIp);

    if (onlyLocal) {
      setStatus('running locally...', 'info');
      appendOutput('> Executing (' + code.length + ' bytes) from ' + activeTab, 'log');
      api('POST', '/api/exec', { code: code }).then(function(res) {
        if (res.status === 200 && res.data.status === 'ok') { setStatus('success', 'ok'); appendOutput('OK', 'ok'); }
        else { setStatus('error', 'err'); appendOutput('ERROR: ' + (res.data.error || 'unknown'), 'err'); }
      });
    } else {
      setStatus('running on ' + targets.length + ' device(s)...', 'info');
      appendOutput('> Mesh exec (' + code.length + ' bytes) \u2192 ' + targets.join(', '), 'log');
      api('POST', '/api/mesh/exec', { code: code, targets: targets }).then(function(res) {
        if (res.status === 200 && res.data.results) {
          var allOk = true;
          for (var ip in res.data.results) {
            var r = res.data.results[ip];
            var rname = r.name || ip;
            if (r.status === 'ok') { appendOutput(rname + ': OK', 'ok'); }
            else { appendOutput(rname + ': ' + r.status + (r.http_code ? ' (HTTP ' + r.http_code + ')' : ''), 'err'); allOk = false; }
          }
          setStatus(allOk ? 'all succeeded' : 'some failed', allOk ? 'ok' : 'err');
        } else {
          setStatus('mesh exec failed', 'err');
          appendOutput('ERROR: ' + (res.data.error || 'unknown'), 'err');
        }
      });
    }
  });

  // ── Save button ──
  // Always routes through /api/mesh/scripts which handles both local + remote.
  // This ensures the host device also gets a checkmark in the devices panel.
  document.getElementById('btn-save').addEventListener('click', function() {
    var name = nameInput.value.trim();
    if (!name) { setStatus('enter a name', 'err'); return; }
    if (!/^[a-zA-Z0-9_-]+$/.test(name)) { setStatus('name: alphanumeric/-/_', 'err'); return; }
    var code, type;
    if (activeTab === 'blockly') { var wsState = Blockly.serialization.workspaces.save(workspace); code = JSON.stringify(wsState); type = 'bly'; }
    else { code = cm.getValue(); type = 'js'; }
    var cronStr = document.getElementById('save-cron').value.trim();

    var targets = Array.from(selectedDeviceIps);

    // Clear previous save icons
    document.querySelectorAll('[id^="save-icon-"]').forEach(function(el) { el.style.display = 'none'; el.textContent = ''; });

    setStatus('saving to ' + (targets.length || 1) + ' device(s)...', 'info');
    document.getElementById('btn-save').textContent = 'Saving...';
    var payload = { name: name, code: code, type: type, cron: cronStr };
    if (targets.length > 0) payload.targets = targets;
    api('POST', '/api/mesh/scripts', payload).then(function(res) {
      document.getElementById('btn-save').textContent = 'Save';
      if (res.status === 200 && res.data.results) {
        var allOk = true;
        for (var ip in res.data.results) {
          var r = res.data.results[ip];
          var iconEl = document.getElementById('save-icon-' + ip.replace(/\./g, '-'));
          if (iconEl) {
            iconEl.textContent = (r.status === 'ok') ? '\u2705' : '\u274c';
            iconEl.style.display = 'inline';
          }
          if (r.status !== 'ok') allOk = false;
        }
        setStatus(allOk ? 'saved to all' : 'some saves failed', allOk ? 'ok' : 'err');
        loadScriptsList().then(function() { dropdown.value = name; });
      } else {
        setStatus('save failed: ' + (res.data.error || ''), 'err');
      }
    });
  });

  // ── Delete button ──
  document.getElementById('btn-delete').addEventListener('click', function() {
    var name = nameInput.value.trim();
    if (!name) { setStatus('no script selected', 'err'); return; }
    if (!confirm('Delete "' + name + '"?')) return;
    api('DELETE', '/api/scripts/' + encodeURIComponent(name)).then(function(res) {
      if (res.status === 200) { setStatus('deleted: ' + name, 'ok'); nameInput.value = ''; cm.setValue(''); workspace.clear(); loadScriptsList(); }
      else { setStatus('delete failed', 'err'); }
    });
  });

  // ── Copy & keyboard shortcut ──
  btnCopy.addEventListener('click', function() { var code = generateElkCode(); cm.setValue(code); switchTab('code'); setStatus('copied to editor', 'ok'); });
  document.getElementById('btn-copy-code').addEventListener('click', function() { if (navigator.clipboard) { navigator.clipboard.writeText(lastGenerated); setStatus('copied!', 'ok'); } });
  document.addEventListener('keydown', function(ev) { if (ev.keyCode === 13 && (ev.metaKey || ev.ctrlKey)) { ev.preventDefault(); document.getElementById('btn-run').click(); } });

  loadScriptsList();
  setStatus('ready', 'info');

  // ════════════════════════════════════════════════════════════════════════
  //  §7  Cron hint
  // ════════════════════════════════════════════════════════════════════════

  var cronInput = document.getElementById('save-cron');
  var cronHint = document.getElementById('cron-hint');

  function describeCron(expr) {
    if (!expr) return '';
    var p = expr.trim().split(/\s+/);
    if (p.length < 5) return '\u26a0 need 5 fields: MIN HR DOM MON DOW';
    if (p.length > 5) return '\u26a0 too many fields (expected 5)';
    var mn = p[0], hr = p[1], dom = p[2], mon = p[3], dow = p[4];
    function validField(f, lo, hi) {
      var parts = f.split(',');
      for (var i = 0; i < parts.length; i++) {
        var t = parts[i], step = t.split('/');
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

  // ════════════════════════════════════════════════════════════════════════
  //  §8  Mesh devices panel + live clock
  // ════════════════════════════════════════════════════════════════════════

  var devicesList = document.getElementById('devices-list');
  var knownDevices = [];
  var selectedDeviceIps = new Set();

  function loadMeshPeers() {
    return api('GET', '/api/status').then(function(res) {
      if (res.status !== 200 || !res.data) return;
      var selfDevice = {
        name: res.data.name || res.data.device_name || 'device',
        device_name: res.data.device_name || res.data.name || 'device',
        ip: res.data.ip || '',
        mac: res.data.mac || '',
        self: true,
        time: res.data.time || null,
        date: res.data.date || null,
        epoch: res.data.epoch || null,
        heap_free: res.data.heap_free,
        uptime_ms: res.data.uptime_ms,
        ws_connected: res.data.ws_connected,
        mqtt_connected: res.data.mqtt_connected,
        wifi_rssi: res.data.wifi_rssi,
        mesh_peers: res.data.mesh_peers,
        setup_mode: res.data.setup_mode
      };
      knownDevices = [selfDevice].concat(Array.isArray(res.data.peers) ? res.data.peers : []);
      if (selectedDeviceIps.size === 0) {
        knownDevices.forEach(function(d) { if (d.self) selectedDeviceIps.add(d.ip); });
      }
      selectedDeviceIps.forEach(function(ip) {
        if (!knownDevices.some(function(d) { return d.ip === ip; })) selectedDeviceIps.delete(ip);
      });
      renderDevices();
      updateRemoteApi();
    });
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
      info.style.flex = '1';
      info.style.minWidth = '0';

      var nameEl = document.createElement('div');
      nameEl.className = 'dev-name';
      nameEl.textContent = d.name;

      var ipRow = document.createElement('div');
      ipRow.className = 'dev-ip';
      ipRow.appendChild(document.createTextNode(d.ip + ' '));

      var clockSpan = document.createElement('span');
      clockSpan.className = 'clock-tick';
      clockSpan.style.color = '#aaa';
      if (d.time) {
        clockSpan.textContent = '\u23f0 ' + d.time;
        clockSpan.setAttribute('data-epoch', d.epoch || '');
      }
      ipRow.appendChild(clockSpan);

      var saveIcon = document.createElement('span');
      saveIcon.id = 'save-icon-' + d.ip.replace(/\./g, '-');
      saveIcon.style.display = 'none';
      saveIcon.style.marginLeft = '4px';
      ipRow.appendChild(saveIcon);

      var tagEl = document.createElement('div');
      tagEl.className = d.self ? 'dev-self' : 'dev-remote';
      tagEl.textContent = d.self ? '(this device)' : 'peer';

      info.appendChild(nameEl);
      info.appendChild(ipRow);
      info.appendChild(tagEl);
      div.appendChild(cb);
      div.appendChild(info);
      devicesList.appendChild(div);
    });
  }

  // Tick every clock element once per second.
  // We increment the HH:MM:SS string directly instead of converting
  // through Date(), which would use the browser timezone rather than
  // the device's configured timezone.
  setInterval(function() {
    document.querySelectorAll('.clock-tick').forEach(function(el) {
      var epochStr = el.getAttribute('data-epoch');
      if (!epochStr || isNaN(epochStr)) return;
      el.setAttribute('data-epoch', String(parseInt(epochStr, 10) + 1));
      var text = el.textContent.replace('\u23f0 ', '');
      var parts = text.split(':');
      if (parts.length !== 3) return;
      var hh = parseInt(parts[0], 10);
      var mm = parseInt(parts[1], 10);
      var ss = parseInt(parts[2], 10) + 1;
      if (ss >= 60) { ss = 0; mm++; }
      if (mm >= 60) { mm = 0; hh++; }
      if (hh >= 24) { hh = 0; }
      el.textContent = '\u23f0 '
        + String(hh).padStart(2, '0') + ':'
        + String(mm).padStart(2, '0') + ':'
        + String(ss).padStart(2, '0');
    });
  }, 1000);

  // ════════════════════════════════════════════════════════════════════════
  //  §9  Remote API panel
  // ════════════════════════════════════════════════════════════════════════

  var localApiHtml = document.getElementById('info-panel').innerHTML;

  function resetLocalApiPanel() {
    document.getElementById('info-panel').innerHTML = localApiHtml;
  }

  function updateRemoteApi() {
    var panel = document.getElementById('info-panel');
    var selected = Array.from(selectedDeviceIps);
    var remoteIps = selected.filter(function(ip) {
      var d = knownDevices.find(function(dev) { return dev.ip === ip; });
      return d && !d.self;
    });

    if (remoteIps.length !== 1) { resetLocalApiPanel(); return; }

    var ip = remoteIps[0];
    var dev = knownDevices.find(function(d) { return d.ip === ip; });
    panel.innerHTML = '<h4>API: ' + (dev ? dev.name : ip) + '</h4><div style="color:#888">Loading...</div>';
    api('GET', '/api/mesh/api?ip=' + encodeURIComponent(ip)).then(function(res) {
      if (res.status === 200 && res.data.objects) {
        var html = '<h4>API: ' + (dev ? dev.name : ip) + '</h4>';
        for (var objName in res.data.objects) {
          html += '<h4>' + objName + '</h4><ul>';
          var obj = res.data.objects[objName];
          if (obj.methods) {
            for (var m in obj.methods) {
              var meth = obj.methods[m];
              var sig = objName + '.' + m + '(' + (meth.args||[]).join(', ') + ')';
              html += '<li><code>' + sig + '</code>' + (meth.ret ? ' \u2192 ' + meth.ret : '') + (meth.desc ? ' \u2014 ' + meth.desc : '') + '</li>';
            }
          }
          if (obj.constants) { html += '<li><code>' + obj.constants.join(', ') + '</code></li>'; }
          html += '</ul>';
        }
        if (res.data.globals) {
          html += '<h4>Globals</h4><ul>';
          for (var g in res.data.globals) {
            var gl = res.data.globals[g];
            if (gl.args) html += '<li><code>' + g + '(' + gl.args.join(', ') + ')</code>' + (gl.desc ? ' \u2014 ' + gl.desc : '') + '</li>';
            else if (gl.value !== undefined) html += '<li><code>' + g + '</code> = ' + gl.value + '</li>';
          }
          html += '</ul>';
        }
        panel.innerHTML = html;
      } else {
        panel.innerHTML = '<h4>API: ' + (dev ? dev.name : ip) + '</h4><div style="color:#f66">Failed to fetch API</div>';
      }
    });
  }

  document.getElementById('btn-refresh-peers').addEventListener('click', loadMeshPeers);
  setInterval(loadMeshPeers, 60000);
  loadMeshPeers();

  // ════════════════════════════════════════════════════════════════════════
  //  §10  Settings tab
  // ════════════════════════════════════════════════════════════════════════

  var cfgStatus = document.getElementById('cfg-status');
  function setCfgStatus(msg, cls) { cfgStatus.textContent = msg; cfgStatus.className = 'status ' + cls; }

  function loadSettings() {
    api('GET', '/api/config').then(function(res) {
      if (res.status !== 200) return;
      document.getElementById('cfg-device-name').value = res.data.device_name || '';
      document.getElementById('cfg-ws-enabled').checked = !!res.data.ws_enabled;
      document.getElementById('cfg-ws-host').value = res.data.ws_host || '';
      document.getElementById('cfg-ws-port').value = res.data.ws_port || 443;
      document.getElementById('cfg-ws-path').value = res.data.ws_path || '/kairos/{device_id}';
      document.getElementById('cfg-ws-ssl').checked = res.data.ws_ssl !== false;
      document.getElementById('cfg-ntp-server').value = res.data.ntp_server || '';
      document.getElementById('cfg-tz-info').value = res.data.tz_info || '';
      document.getElementById('cfg-mqtt-enabled').checked = !!res.data.mqtt_enabled;
      document.getElementById('cfg-mqtt-broker').value = res.data.mqtt_broker || '';
      document.getElementById('cfg-mqtt-port').value = res.data.mqtt_port || 1883;
      document.getElementById('cfg-mqtt-user').value = '';
      document.getElementById('cfg-mqtt-pass').value = '';
      document.getElementById('cfg-mqtt-prefix').value = res.data.mqtt_topic_prefix || 'kairos';
    });
    api('GET', '/api/status').then(function(st) {
      if (st.status !== 200) return;
      document.getElementById('st-ip').textContent = 'IP: ' + (st.data.ip || '?');
      document.getElementById('st-heap').textContent = 'Heap: ' + (st.data.heap_free || '?') + ' bytes';
      document.getElementById('st-uptime').textContent = 'Uptime: ' + Math.round((st.data.uptime_ms || 0) / 1000) + 's';
      document.getElementById('st-ws').textContent = 'WS Client: ' + (st.data.ws_connected ? 'connected' : 'disconnected');
      document.getElementById('st-mqtt').textContent = 'MQTT: ' + (st.data.mqtt_connected ? 'connected' : 'disconnected');
      document.getElementById('st-rssi').textContent = 'WiFi RSSI: ' + (st.data.wifi_rssi || '?') + ' dBm';
      document.getElementById('st-time').textContent = 'Device Time: ' + (st.data.time ? st.data.date + ' ' + st.data.time : 'not synced');
    });
  }

  document.getElementById('btn-cfg-save').addEventListener('click', function() {
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
    api('POST', '/api/config', cfg).then(function(res) {
      setCfgStatus(res.status === 200 ? 'Saved! Restart device to apply WS changes.' : 'Save failed', res.status === 200 ? 'ok' : 'err');
    });
  });

  // ════════════════════════════════════════════════════════════════════════
  //  §11  Draggable splitters
  // ════════════════════════════════════════════════════════════════════════

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
      if (dragging === 'vb' && blocklyLayout) {
        var rect = blocklyLayout.getBoundingClientRect();
        var newW = rect.right - e.clientX - 3;
        if (newW < 120) newW = 120; if (newW > rect.width - 200) newW = rect.width - 200;
        codePreview.style.width = newW + 'px';
        Blockly.svgResize(workspace);
      } else if (dragging === 'vc' && codeLayout) {
        var rect = codeLayout.getBoundingClientRect();
        var newW = rect.right - e.clientX - 3;
        if (newW < 150) newW = 150; if (newW > rect.width - 200) newW = rect.width - 200;
        infoPanel.style.width = newW + 'px';
        if (cm) cm.refresh();
      } else if (dragging === 'h') {
        var bh = document.body.getBoundingClientRect().height;
        var newH = bh - e.clientY - 3;
        if (newH < 40) newH = 40; if (newH > bh - 200) newH = bh - 200;
        outputEl.style.height = newH + 'px';
        if (activeTab === 'blockly') Blockly.svgResize(workspace);
        else if (cm) cm.refresh();
      }
    });
    document.addEventListener('mouseup', function() { dragging = null; });
  })();

  window.addEventListener('resize', function() {
    if (activeTab === 'blockly') Blockly.svgResize(workspace);
    else if (cm) cm.refresh();
  });
})();
)rawjs";
