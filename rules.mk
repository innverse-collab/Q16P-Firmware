# No real bootloader - bare silicon, flashed via SWD at 0x08000000 (see README)
BOOTLOADER = custom
MCU_LDSCRIPT = STM32F103xB_no_boot

# Features (mirrors keyboard.json "features", matches the AtMega32u4 source)
BOOTMAGIC_ENABLE = yes
MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
NKRO_ENABLE = yes
BACKLIGHT_ENABLE = yes
RGBLIGHT_ENABLE = no
AUDIO_ENABLE = no
