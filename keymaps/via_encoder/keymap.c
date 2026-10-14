// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Ported from Q16P-Firmware-AtMega32u4/keymaps/via_encoder to the
// Macropad_4x4_stm32 (STM32F103C8) PCB. Keymap/layer/encoder logic is
// unchanged from the AtMega32u4 source - only GPIO pin names were remapped
// to match the new schematic. Two things present on the AtMega32u4 board
// have no equivalent pin on this PCB and were therefore dropped rather than
// silently faked:
//   - SPACEBAR_PIN (F7 on AtMega32u4): this PCB's Switch.kicad_sch has no
//     21st direct-GPIO switch outside the 4x5 (20-key) matrix.
//   - ENC_SW_2_PIN (F0 on AtMega32u4): encoder 2's switch (SW3, pin S1) is
//     explicitly marked no-connect on this schematic; only encoder 1's
//     switch (ENC_SW_1 / PC15) is wired.

#include QMK_KEYBOARD_H

// Define layers
enum layers {
    _BASE,
    _LAY_1,
    _LAY_2,
    _LAY_3
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┬───┐
     * │   │   │   │   │   │
     * │ / │ 7 │ 8 │ 9 │L0 │
     * │ * │ 4 │ 5 │ 6 │L1 │
     * │ - │ 1 │ 2 │ 3 │L2 │
     * │ + │ 0 │ . │Ent│L3 │
     * └───┴───┴───┴───┴───┘
     */
    [_BASE] = LAYOUT(
        KC_PSLS,    KC_P7,   KC_P8,   KC_P9,    KC_NO,
        KC_PAST,    KC_P4,   KC_P5,   KC_P6,    TO(_LAY_1),
        KC_PMNS,    KC_P1,   KC_P2,   KC_P3,    TO(_LAY_2),
        KC_PPLS,    KC_P0,   KC_PDOT, KC_PENT,  TO(_LAY_3)
    ),

    [_LAY_1] = LAYOUT(
        KC_D,       KC_A,    KC_B,    KC_C,     TO(_BASE),
        KC_H,       KC_E,    KC_F,    KC_G,     KC_NO,
        KC_L,       KC_I,    KC_J,    KC_K,     TO(_LAY_2),
        KC_P,       KC_M,    KC_N,    KC_O,     TO(_LAY_3)
    ),

    [_LAY_2] = LAYOUT(
        KC_R,       KC_Q,    KC_W,    KC_E,     TO(_BASE),
        KC_I,       KC_T,    KC_Y,    KC_U,     TO(_LAY_1),
        KC_RBRC,    KC_O,    KC_P,    KC_LBRC,  KC_NO,
        KC_SLSH,    KC_SCLN, KC_QUOT, KC_BSLS,  TO(_LAY_3)
    ),

    [_LAY_3] = LAYOUT(
        KC_F4,      KC_F1,   KC_F2,   KC_F3,    TO(_BASE),
        KC_F8,      KC_F5,   KC_F6,   KC_F7,    TO(_LAY_1),
        KC_F12,     KC_F9,   KC_F10,  KC_F11,   TO(_LAY_2),
        KC_PGDN,    KC_HOME, KC_END,  KC_PGUP,  KC_NO
    )
};


// LED pins (layer indicators - Mode_1..Mode_4 nets, driven through
// MMBT2222 transistors, active HIGH)
#define LED1_PIN B15  // Mode_1
#define LED2_PIN B14  // Mode_2
#define LED3_PIN B13  // Mode_3
#define LED4_PIN B12  // Mode_4

// Encoder 1 switch pin (direct GPIO, active low)
#define ENC_SW_1_PIN C15

// Per-switch backlight LED enable (LED_Drive net -> R34 -> Q5 gate; Q5 is an
// N-channel MOSFET low-side switch for the GND-M return rail shared by all
// 20 per-switch LEDs). This is driven by QMK's PWM backlight feature
// (BACKLIGHT_PIN/BACKLIGHT_PWM_* in config.h), not raw GPIO calls here, so
// that brightness can be a level 0..BACKLIGHT_LEVELS instead of just on/off.
// Defaults to full brightness on a fresh board (QMK's stock EEPROM default);
// brightness is adjusted at runtime by the Layer 2 (_LAY_1) encoder 1
// rotation handler below.

void keyboard_post_init_user(void) {
    // Set LED pins as output
    gpio_set_pin_output(LED1_PIN);
    gpio_set_pin_output(LED2_PIN);
    gpio_set_pin_output(LED3_PIN);
    gpio_set_pin_output(LED4_PIN);
    gpio_write_pin_high(LED1_PIN);
    gpio_write_pin_low(LED2_PIN);
    gpio_write_pin_low(LED3_PIN);
    gpio_write_pin_low(LED4_PIN);

    // Set encoder 1 switch pin as input with pullup (active low)
    gpio_set_pin_input_high(ENC_SW_1_PIN);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // Turn off LEDs first
    gpio_write_pin_low(LED1_PIN);
    gpio_write_pin_low(LED2_PIN);
    gpio_write_pin_low(LED3_PIN);
    gpio_write_pin_low(LED4_PIN);

    switch (get_highest_layer(state)) {
        case _BASE:
            gpio_write_pin_high(LED1_PIN);  // LED1 on for base
            break;
        case _LAY_1:
            gpio_write_pin_high(LED2_PIN);  // LED2 on for Layer 1
            break;
        case _LAY_2:
            gpio_write_pin_high(LED3_PIN);  // LED3 on for Layer 1
            break;
        case _LAY_3:
            gpio_write_pin_high(LED4_PIN);  // LED3 on for Layer 1
            break;
    }
    return state;
}

// Custom function for base layer encoder switch (Layer 1 / _BASE) - opens
// the Innverse VIA configurator. Hardcoded: encoder switch clicks are read
// via direct GPIO in matrix_scan_user() below, bypassing VIA's dynamic
// keymap entirely, so this is not editable from VIA - only the encoder's
// rotation (volume/mute, or brightness on Layer 2) is VIA-programmable.
void base_layer_encoder_switch_sequence(void) {
    register_code(KC_LGUI);
    register_code(KC_R);
    unregister_code(KC_R);
    unregister_code(KC_LGUI);

    // 100ms delay
    wait_ms(100);

    // Type the URL
    SEND_STRING("https://www.innverse.in/via-configurator");

    // 100ms delay
    wait_ms(100);

    // Press Enter
    register_code(KC_ENT);
    unregister_code(KC_ENT);
}

// Custom function for layer 1 encoder switch (Ctrl + Z for UNDO)
void layer_1_encoder_switch_sequence(void) {
    register_code(KC_LCTL);
    register_code(KC_Z);
    unregister_code(KC_Z);
    unregister_code(KC_LCTL);
}

// Custom function for layer 2 encoder switch (Ctrl + Y for REDO)
void layer_2_encoder_switch_sequence(void) {
    register_code(KC_LCTL);
    register_code(KC_Y);
    unregister_code(KC_Y);
    unregister_code(KC_LCTL);
}

// Custom function for layer 3 encoder switch (Windows + L to lock)
void layer_3_encoder_switch_sequence(void) {
    register_code(KC_LGUI);
    register_code(KC_L);
    unregister_code(KC_L);
    unregister_code(KC_LGUI);
}

// Encoder 1 switch mapping - programmable per layer
const uint16_t PROGMEM encoder_1_switch_map[] = {
    [_BASE]  = KC_NO,   // Custom sequence for base layer (handled separately)
    [_LAY_1] = KC_NO,   // Custom sequence for layer 1 (handled separately) UNDO
    [_LAY_2] = KC_NO,   // Custom sequence for layer 2 (handled separately) REDO
    [_LAY_3] = KC_NO    // Custom sequence for layer 3 (handled separately) Lock Screen
};

// Encoder 1 switch handling - override matrix scan
void matrix_scan_user(void) {
    static bool enc_sw_1_pressed = false;
    static uint32_t enc_sw_1_debounce_time = 0;

    uint32_t current_time = timer_read32();

    // Get current layer for encoder switch mapping
    uint8_t current_layer = get_highest_layer(layer_state);

    // Encoder 1 switch handling (active LOW - pullup resistor) with debouncing
    bool current_enc_sw_1_state = !gpio_read_pin(ENC_SW_1_PIN);

    if (current_enc_sw_1_state && !enc_sw_1_pressed) {
        if (current_time - enc_sw_1_debounce_time > 5) { // 5ms debounce
            if (current_layer == _BASE) {
                // Execute custom sequence for base layer
                base_layer_encoder_switch_sequence();
            } else if (current_layer == _LAY_1) {
                // Execute custom sequence for layer 1 (UNDO)
                layer_1_encoder_switch_sequence();
            } else if (current_layer == _LAY_2) {
                // Execute custom sequence for layer 2 (REDO)
                layer_2_encoder_switch_sequence();
            } else if (current_layer == _LAY_3) {
                // Execute custom sequence for layer 3 (Lock Screen)
                layer_3_encoder_switch_sequence();
            } else if (current_layer < sizeof(encoder_1_switch_map)/sizeof(encoder_1_switch_map[0]) &&
                       encoder_1_switch_map[current_layer] != KC_NO) {
                register_code(encoder_1_switch_map[current_layer]);
            }
            enc_sw_1_pressed = true;
        }
    } else if (!current_enc_sw_1_state && enc_sw_1_pressed) {
        if (current_time - enc_sw_1_debounce_time > 5) { // 5ms debounce
            if (current_layer != _BASE && current_layer != _LAY_1 &&
                current_layer != _LAY_2 && current_layer != _LAY_3 &&
                current_layer < sizeof(encoder_1_switch_map)/sizeof(encoder_1_switch_map[0]) &&
                encoder_1_switch_map[current_layer] != KC_NO) {
                unregister_code(encoder_1_switch_map[current_layer]);
            }
            enc_sw_1_pressed = false;
        }
    } else if (current_enc_sw_1_state != enc_sw_1_pressed) {
        enc_sw_1_debounce_time = current_time;
    }
}

// Encoder rotation - intercept Layer 2 (_LAY_1) encoder 1 to hardcode it as
// backlight brightness control, instead of the usual VIA-programmable
// keycode from encoder_map[].
//
// This does NOT use encoder_update_user() - with ENCODER_MAP_ENABLE on
// (required for VIA to expose encoder rotation at all), QMK's encoder task
// (quantum/encoder.c: encoder_handle_queue()) routes rotation straight
// through action_exec()/process_record() using the dynamic keymap, and
// never calls encoder_update_kb()/encoder_update_user() - that hook is
// simply dead code whenever ENCODER_MAP_ENABLE is set. The actual
// interception point is here, in process_record_user(): by the time this
// runs, QMK has already resolved VIA's stored keycode for the encoder
// event, but nothing has acted on it yet - returning false drops it.
//
// Caveat: VIA's UI still *shows* the Layer 2 / encoder 1 cell as editable
// (VIA has no concept of a "locked" cell) - assigning something there just
// has no effect, since this intercepts the event before that keycode would
// ever be processed.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (IS_ENCODEREVENT(record->event)) {
        uint8_t enc_index = record->event.key.col;
        bool    clockwise = (record->event.key.row == KEYLOC_ENCODER_CW);

        if (enc_index == 0 && get_highest_layer(layer_state) == _LAY_1) {
            if (record->event.pressed) {
                if (clockwise) {
                    backlight_increase();
                } else {
                    backlight_decrease();
                }
            }
            return false;
        }
    }

    return true;
}

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    // Encoder 1 (index 0) here is intercepted in process_record_user()
    // above and never actually fires whatever keycode ends up resolved
    // from this table (or from VIA's copy of it) - left as KC_NO, rather
    // than VOLD/VOLU, just so the firmware default isn't misleading. Note
    // this doesn't retroactively change anything already saved in VIA -
    // VIA's dynamic keymap lives in EEPROM independently of this array
    // once initialized; the interception is what actually makes this cell
    // inert, not this default value.
    [_LAY_1] = { ENCODER_CCW_CW(KC_NO, KC_NO),      ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  }
};
#endif
