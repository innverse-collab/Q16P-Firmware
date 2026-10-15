// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Ported from Q16P-Firmware-AtMega32u4/keymaps/via_encoder - see README.md for the pin map and hardware differences.

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


// LED pins (layer indicators, Mode_1..Mode_4 nets, active HIGH)
#define LED1_PIN B15  // Mode_1
#define LED2_PIN B14  // Mode_2
#define LED3_PIN B13  // Mode_3
#define LED4_PIN B12  // Mode_4

// Encoder 1 switch pin (direct GPIO, active low)
#define ENC_SW_1_PIN C15

// Per-switch backlight (LED_Drive/PA8) - PWM backlight feature, see README

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
            gpio_write_pin_high(LED3_PIN);  // LED3 on for Layer 2
            break;
        case _LAY_3:
            gpio_write_pin_high(LED4_PIN);  // LED4 on for Layer 3
            break;
    }
    return state;
}

// Base layer (_BASE) encoder click - opens the Innverse VIA configurator. OS-aware (see README); not VIA-programmable.
void base_layer_encoder_switch_sequence(void) {
    switch (detected_host_os()) {
        case OS_MACOS:
        case OS_IOS:
            // Cmd+Space (Spotlight)
            register_code(KC_LGUI);
            register_code(KC_SPC);
            unregister_code(KC_SPC);
            unregister_code(KC_LGUI);
            wait_ms(300);
            SEND_STRING("https://www.innverse.in/via-configurator");
            wait_ms(100);
            register_code(KC_ENT);
            unregister_code(KC_ENT);
            break;

        case OS_LINUX:
            // Alt+F2, then xdg-open <url>
            register_code(KC_LALT);
            register_code(KC_F2);
            unregister_code(KC_F2);
            unregister_code(KC_LALT);
            wait_ms(250);
            SEND_STRING("xdg-open https://www.innverse.in/via-configurator");
            wait_ms(100);
            register_code(KC_ENT);
            unregister_code(KC_ENT);
            break;

        case OS_WINDOWS:
        case OS_UNSURE:
        default:
            // Win+R (Run dialog)
            register_code(KC_LGUI);
            register_code(KC_R);
            unregister_code(KC_R);
            unregister_code(KC_LGUI);
            wait_ms(250);
            SEND_STRING("https://www.innverse.in/via-configurator");
            wait_ms(100);
            register_code(KC_ENT);
            unregister_code(KC_ENT);
            break;
    }
}

// Layer 2 (_LAY_1) encoder click - screenshot (area/region select). OS-aware (see README).
void layer_1_encoder_switch_sequence(void) {
    switch (detected_host_os()) {
        case OS_MACOS:
        case OS_IOS:
            // Cmd+Shift+4
            register_code(KC_LGUI);
            register_code(KC_LSFT);
            register_code(KC_4);
            unregister_code(KC_4);
            unregister_code(KC_LSFT);
            unregister_code(KC_LGUI);
            break;

        case OS_LINUX:
            // Shift+PrtScn
            register_code(KC_LSFT);
            register_code(KC_PSCR);
            unregister_code(KC_PSCR);
            unregister_code(KC_LSFT);
            break;

        case OS_WINDOWS:
        case OS_UNSURE:
        default:
            // Win+Shift+S
            register_code(KC_LGUI);
            register_code(KC_LSFT);
            register_code(KC_S);
            unregister_code(KC_S);
            unregister_code(KC_LSFT);
            unregister_code(KC_LGUI);
            break;
    }
}

// Layer 3 (_LAY_2) encoder click - opens a terminal. OS-aware (see README).
void layer_2_encoder_switch_sequence(void) {
    switch (detected_host_os()) {
        case OS_MACOS:
        case OS_IOS:
            register_code(KC_LGUI);
            register_code(KC_SPC);
            unregister_code(KC_SPC);
            unregister_code(KC_LGUI);
            wait_ms(300);
            SEND_STRING("Terminal");
            wait_ms(100);
            register_code(KC_ENT);
            unregister_code(KC_ENT);
            break;

        case OS_LINUX:
            register_code(KC_LCTL);
            register_code(KC_LALT);
            register_code(KC_T);
            unregister_code(KC_T);
            unregister_code(KC_LALT);
            unregister_code(KC_LCTL);
            break;

        case OS_WINDOWS:
        case OS_UNSURE:
        default:
            register_code(KC_LGUI);
            register_code(KC_R);
            unregister_code(KC_R);
            unregister_code(KC_LGUI);
            wait_ms(250);
            SEND_STRING("cmd");
            wait_ms(100);
            register_code(KC_ENT);
            unregister_code(KC_ENT);
            break;
    }
}

// Layer 4 (_LAY_3) encoder click - locks the screen. OS-aware (see README).
void layer_3_encoder_switch_sequence(void) {
    switch (detected_host_os()) {
        case OS_MACOS:
        case OS_IOS:
            register_code(KC_LCTL);
            register_code(KC_LGUI);
            register_code(KC_Q);
            unregister_code(KC_Q);
            unregister_code(KC_LGUI);
            unregister_code(KC_LCTL);
            break;

        case OS_WINDOWS:
        case OS_LINUX:
        case OS_UNSURE:
        default:
            register_code(KC_LGUI);
            register_code(KC_L);
            unregister_code(KC_L);
            unregister_code(KC_LGUI);
            break;
    }
}

// Encoder 1 switch mapping - programmable per layer
const uint16_t PROGMEM encoder_1_switch_map[] = {
    [_BASE]  = KC_NO,   // Custom sequence for base layer (handled separately)
    [_LAY_1] = KC_NO,   // Custom sequence for layer 1 (handled separately) Screenshot
    [_LAY_2] = KC_NO,   // Custom sequence for layer 2 (handled separately) Open Terminal
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
                // Execute custom sequence for layer 1 (Screenshot)
                layer_1_encoder_switch_sequence();
            } else if (current_layer == _LAY_2) {
                // Execute custom sequence for layer 2 (Open Terminal)
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

// Layer 2 (_LAY_1) encoder 1 rotation -> backlight brightness, hardcoded (not VIA-programmable - see README).
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
    [_LAY_1] = { ENCODER_CCW_CW(KC_NO, KC_NO),      ENCODER_CCW_CW(KC_MUTE, KC_F7)  }, // encoder 1 overridden in process_record_user()
    [_LAY_2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  }
};
#endif
