#pragma once
#include_next <mcuconf.h>

#undef STM32_PLLXTPRE
#define STM32_PLLXTPRE STM32_PLLXTPRE_DIV1

#undef STM32_PLLMUL_VALUE
#define STM32_PLLMUL_VALUE 9

#undef STM32_HSECLK
#define STM32_HSECLK 8000000

// PA8 (backlight/LED_Drive) is TIM1_CH1 - enable TIM1 for PWM backlight.
#undef STM32_PWM_USE_TIM1
#define STM32_PWM_USE_TIM1 TRUE
