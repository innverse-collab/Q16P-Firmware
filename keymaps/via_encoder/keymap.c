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
    writePinHigh(LED1_PIN);
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

// Custom function for base layer encoder switch
void base_layer_encoder_switch_sequence(void) {
    register_code(KC_LGUI);
    register_code(KC_R);
    unregister_code(KC_R);
    unregister_code(KC_LGUI);
    
    // 100ms delay
    wait_ms(100);
    
    // Type "www.caniusevia.com"
    SEND_STRING("www.caniusevia.com");
    
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

// Spacebar and encoder 1 switch handling - override matrix scan
void matrix_scan_user(void) {
    static bool spacebar_pressed = false;
    static bool enc_sw_1_pressed = false;
    static uint32_t spacebar_debounce_time = 0;
    static uint32_t enc_sw_1_debounce_time = 0;
    
    // Spacebar handling (active HIGH - pulldown resistor) with debouncing
    bool current_spacebar_state = readPin(SPACEBAR_PIN);
    uint32_t current_time = timer_read32();
    
    if (current_spacebar_state && !spacebar_pressed) {
        if (current_time - spacebar_debounce_time > 5) { // 5ms debounce
            register_code(KC_SPC);
            spacebar_pressed = true;
        }
    } else if (!current_spacebar_state && spacebar_pressed) {
        if (current_time - spacebar_debounce_time > 5) { // 5ms debounce
            unregister_code(KC_SPC);
            spacebar_pressed = false;
        }
    } else if (current_spacebar_state != spacebar_pressed) {
        spacebar_debounce_time = current_time;
    }
    
    // Get current layer for encoder switch mapping
    uint8_t current_layer = get_highest_layer(layer_state);
    
    // Encoder 1 switch handling (active LOW - pullup resistor) with debouncing
    bool current_enc_sw_1_state = !readPin(ENC_SW_1_PIN);
    
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

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_1] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  },
    [_LAY_3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_MUTE, KC_F7)  }
};
#endif
