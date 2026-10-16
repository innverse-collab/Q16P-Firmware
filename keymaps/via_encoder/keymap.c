// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Ported from Q16P-Firmware-AtMega32u4/keymaps/via_encoder - see README.md for the pin map and hardware differences.

#include QMK_KEYBOARD_H
#include <hal.h>  // raw PWM access for the boot animation's backlight fade

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

// Progressive layer indicator - LED count lit = layer number (see README)
static void set_layer_leds(uint8_t layer) {
    gpio_write_pin(LED1_PIN, layer >= _BASE);
    gpio_write_pin(LED2_PIN, layer >= _LAY_1);
    gpio_write_pin(LED3_PIN, layer >= _LAY_2);
    gpio_write_pin(LED4_PIN, layer >= _LAY_3);
}

// Boot animation timing (see README) - total run time RISE_MS + FALL_MS
#define BOOT_ANIM_RISE_MS        900
#define BOOT_ANIM_FALL_MS        900
#define BOOT_ANIM_LED_STAGGER_MS 120  // gap between each LED starting its fade
#define BOOT_ANIM_LED_FADE_MS    400  // each LED's own fade-in/out duration
#define BOOT_ANIM_PWM_STEPS      8    // software-PWM carrier resolution for the red LEDs (125Hz @ 1ms tick)

// Smoothstep ease (3x^2 - 2x^3), fixed-point: x and return both 0-256
static uint16_t boot_anim_ease(uint16_t x) {
    if (x >= 256) return 256;
    uint32_t x2 = (uint32_t)x * x;
    uint32_t x3 = x2 * x / 256;
    return (uint16_t)((3 * x2 - 2 * x3) / 256);
}

// Brightness fraction (0-256) for LED `i` at time t_ms: staggered fade in during
// the rise phase, staggered fade out (reverse order) during the fall phase.
static uint16_t boot_anim_led_fraction(uint8_t i, uint32_t t_ms) {
    if (t_ms < BOOT_ANIM_RISE_MS) {
        uint32_t start = (uint32_t)i * BOOT_ANIM_LED_STAGGER_MS;
        if (t_ms <= start) return 0;
        uint32_t d = t_ms - start;
        if (d >= BOOT_ANIM_LED_FADE_MS) return 256;
        return boot_anim_ease((uint16_t)(d * 256 / BOOT_ANIM_LED_FADE_MS));
    } else {
        uint32_t fall_t = t_ms - BOOT_ANIM_RISE_MS;
        uint32_t start  = (uint32_t)(3 - i) * BOOT_ANIM_LED_STAGGER_MS;
        if (fall_t <= start) return 256;
        uint32_t d = fall_t - start;
        if (d >= BOOT_ANIM_LED_FADE_MS) return 0;
        return 256 - boot_anim_ease((uint16_t)(d * 256 / BOOT_ANIM_LED_FADE_MS));
    }
}

// Boot animation - staggered fade-in/out LED cascade + a true PWM-duty
// backlight fade (bypasses the 8-level brightness abstraction so the ramp
// is continuous, not steppy). See README for the full writeup.
static void boot_animation(void) {
    pin_t    leds[4]      = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN};
    uint8_t  target_level = get_backlight_level();
    bool     fade_backlight = is_backlight_enabled();
    uint32_t target_duty  = fade_backlight ? (0xFFFFUL * target_level / BACKLIGHT_LEVELS) : 0;

    for (uint32_t t = 0; t <= BOOT_ANIM_RISE_MS + BOOT_ANIM_FALL_MS; t++) {
        if (fade_backlight) {
            uint16_t bl_env = (t < BOOT_ANIM_RISE_MS)
                                   ? boot_anim_ease((uint16_t)(t * 256 / BOOT_ANIM_RISE_MS))
                                   : 256 - boot_anim_ease((uint16_t)((t - BOOT_ANIM_RISE_MS) * 256 / BOOT_ANIM_FALL_MS));
            uint32_t duty = target_duty * bl_env / 256;
            pwmEnableChannel(&BACKLIGHT_PWM_DRIVER, BACKLIGHT_PWM_CHANNEL - 1, PWM_FRACTION_TO_WIDTH(&BACKLIGHT_PWM_DRIVER, 0xFFFFU, duty));
        }

        uint8_t carrier_phase = t % BOOT_ANIM_PWM_STEPS;
        for (uint8_t i = 0; i < 4; i++) {
            uint8_t coarse_duty = (uint8_t)(boot_anim_led_fraction(i, t) * BOOT_ANIM_PWM_STEPS / 256);
            gpio_write_pin(leds[i], carrier_phase < coarse_duty);
        }

        wait_ms(1);
    }

    if (fade_backlight) {
        backlight_set(target_level);  // hand back to the normal backlight level (exact, perceptually-corrected value)
    }
    set_layer_leds(get_highest_layer(layer_state));
}

void keyboard_post_init_user(void) {
    // Set LED pins as output
    gpio_set_pin_output(LED1_PIN);
    gpio_set_pin_output(LED2_PIN);
    gpio_set_pin_output(LED3_PIN);
    gpio_set_pin_output(LED4_PIN);
    gpio_write_pin_low(LED1_PIN);
    gpio_write_pin_low(LED2_PIN);
    gpio_write_pin_low(LED3_PIN);
    gpio_write_pin_low(LED4_PIN);

    // Set encoder 1 switch pin as input with pullup (active low)
    gpio_set_pin_input_high(ENC_SW_1_PIN);

    boot_animation();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    set_layer_leds(get_highest_layer(state));

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

// Encoder 1 switch handling - debounced press edge triggers the per-layer
// sequence above. enc_sw_1_raw_last/enc_sw_1_last_change track the raw pin
// for edge detection; enc_sw_1_pressed is the debounced, confirmed state
// that actually gates firing (and re-arms once the switch is released).
void matrix_scan_user(void) {
    static bool     enc_sw_1_pressed      = false;
    static bool     enc_sw_1_raw_last     = false;
    static uint32_t enc_sw_1_last_change  = 0;

    bool     current_raw  = !gpio_read_pin(ENC_SW_1_PIN); // active low
    uint32_t current_time = timer_read32();

    if (current_raw != enc_sw_1_raw_last) {
        enc_sw_1_raw_last    = current_raw;
        enc_sw_1_last_change = current_time;
    }

    if (current_raw != enc_sw_1_pressed && (current_time - enc_sw_1_last_change) > 5) { // 5ms debounce
        enc_sw_1_pressed = current_raw;

        if (enc_sw_1_pressed) {
            switch (get_highest_layer(layer_state)) {
                case _BASE:
                    base_layer_encoder_switch_sequence();
                    break;
                case _LAY_1:
                    layer_1_encoder_switch_sequence();
                    break;
                case _LAY_2:
                    layer_2_encoder_switch_sequence();
                    break;
                case _LAY_3:
                    layer_3_encoder_switch_sequence();
                    break;
            }
        }
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

// Encoder 2 ships blank (KC_NO) on every layer - intentionally left for the
// user to assign via VIA, rather than hardcoding a rotation action.
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_NO, KC_NO)  },
    [_LAY_1] = { ENCODER_CCW_CW(KC_NO, KC_NO),      ENCODER_CCW_CW(KC_NO, KC_NO)  }, // encoder 1 overridden in process_record_user()
    [_LAY_2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_NO, KC_NO)  },
    [_LAY_3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_NO, KC_NO)  }
};
#endif
