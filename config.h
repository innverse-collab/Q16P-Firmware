#pragma once

#define DYNAMIC_KEYMAP_MACRO_COUNT 128

// Per-switch backlight (LED_Drive net -> PA8 -> Q5 gate). PA8 is TIM1_CH1 on
// STM32F103, so the PWM backlight driver can run on it directly.
#define BACKLIGHT_PIN A8
#define BACKLIGHT_PWM_DRIVER PWMD1
#define BACKLIGHT_PWM_CHANNEL 1
#define BACKLIGHT_LEVELS 8
