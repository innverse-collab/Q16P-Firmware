// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

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
     * │   │   │   │   │   │  (Spacebar handled separately via PF7)
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


// LED pins
#define LED1_PIN F1
#define LED2_PIN F4
#define LED3_PIN F5
#define LED4_PIN F6

// Spacebar pin (direct GPIO)
#define SPACEBAR_PIN F7

// Encoder switch pins (direct GPIO)
#define ENC_SW_1_PIN D6
#define ENC_SW_2_PIN F0

void keyboard_post_init_user(void) {
    // Set LED pins as output
    setPinOutput(LED1_PIN);
    setPinOutput(LED2_PIN);
    setPinOutput(LED3_PIN);
    setPinOutput(LED4_PIN);
    writePinLow(LED1_PIN);
    writePinLow(LED2_PIN);
    writePinLow(LED3_PIN);
    writePinLow(LED4_PIN);
    
    // Set spacebar pin as input (external pulldown resistor)
    setPinInput(SPACEBAR_PIN);
    
    // Set encoder switch pins as input with pullup (active low)
    setPinInputHigh(ENC_SW_1_PIN);
    setPinInputHigh(ENC_SW_2_PIN);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // Turn off LEDs first
    writePinLow(LED1_PIN);
    writePinLow(LED2_PIN);
    writePinLow(LED3_PIN);
    writePinLow(LED4_PIN);

    switch (get_highest_layer(state)) {
        case _BASE:
            writePinHigh(LED1_PIN);  // LED1 on for base
            break;
        case _LAY_1:
            writePinHigh(LED2_PIN);  // LED2 on for Layer 1
            break;
        case _LAY_2:
            writePinHigh(LED3_PIN);  // LED3 on for Layer 1
            break;
        case _LAY_3:
            writePinHigh(LED4_PIN);  // LED3 on for Layer 1
            break;
    }
    return state;
}

// Spacebar and encoder switch handling - override matrix scan
void matrix_scan_user(void) {
    static bool spacebar_pressed = false;
    static bool enc_sw_1_pressed = false;
    static bool enc_sw_2_pressed = false;
    
    // Spacebar handling (active HIGH - pulldown resistor)
    bool current_spacebar_state = readPin(SPACEBAR_PIN);
    if (current_spacebar_state && !spacebar_pressed) {
        register_code(KC_SPC);
        spacebar_pressed = true;
    } else if (!current_spacebar_state && spacebar_pressed) {
        unregister_code(KC_SPC);
        spacebar_pressed = false;
    }
    
    // Encoder 1 switch handling (active LOW - pullup resistor)
    bool current_enc_sw_1_state = !readPin(ENC_SW_1_PIN);
    if (current_enc_sw_1_state && !enc_sw_1_pressed) {
        register_code(KC_ENT);  // You can change this to any key you want
        enc_sw_1_pressed = true;
    } else if (!current_enc_sw_1_state && enc_sw_1_pressed) {
        unregister_code(KC_ENT);
        enc_sw_1_pressed = false;
    }
    
    // Encoder 2 switch handling (active LOW - pullup resistor)
    bool current_enc_sw_2_state = !readPin(ENC_SW_2_PIN);
    if (current_enc_sw_2_state && !enc_sw_2_pressed) {
        register_code(KC_ESC);  // You can change this to any key you want
        enc_sw_2_pressed = true;
    } else if (!current_enc_sw_2_state && enc_sw_2_pressed) {
        unregister_code(KC_ESC);
        enc_sw_2_pressed = false;
    }
}

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_1] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  }
};
#endif
