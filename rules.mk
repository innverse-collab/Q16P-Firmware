# No real bootloader - this board is bare silicon, flashed directly via
# SWD/ST-Link at 0x08000000. "custom" is QMK's mechanism for boards with no
# bootloader to jump to; platforms/chibios/bootloaders/custom.c already
# provides safe no-op stubs for bootloader_jump()/mcu_reset().
BOOTLOADER = custom

# Custom linker script: full 64K flash starting at 0x08000000 (no bootloader
# offset). Found in ./ld/STM32F103xB_no_boot.ld - QMK's build system searches
# the keyboard's own ld/ directory before falling back to board/chibios
# defaults.
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
