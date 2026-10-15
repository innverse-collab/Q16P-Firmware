# Q16P STM32 - 4x5 Macropad (ported from Q16P-Firmware-AtMega32u4)

Direct port of [Q16P-Firmware-AtMega32u4](../Q16P-Firmware-AtMega32u4) to the
STM32F103C8 "Macropad_4x4_stm32" PCB. Keymap, layers, encoder behavior and
LED logic are unchanged from the AtMega32u4 source - only pin names were
remapped to this schematic. No new features, no optimizations (planned for
later).

Pin assignments below were read directly off `Macropad_4x4_stm32.pdf`
(MCU.kicad_sch / Switch.kicad_sch), not assumed.

Verified by compiling both keymaps (`default`, `via_encoder`) against the
`qmk_firmware` checkout at `/root/qmk_firmware` - both build clean.

One mechanical (non-behavioral) change was required to make it compile: the
AtMega32u4 source calls the old camelCase GPIO API
(`setPinOutput`/`writePinHigh`/`writePinLow`/`setPinInputHigh`/`readPin`).
That API was renamed repo-wide in QMK in Feb 2024
(`docs/ChangeLog/20240225.md`) to `gpio_set_pin_output` /
`gpio_write_pin_high` / `gpio_write_pin_low` / `gpio_set_pin_input_high` /
`gpio_read_pin`, and the old names no longer exist at all in this
`qmk_firmware` checkout (not even for AVR) - so the AtMega32u4 source as
currently written would also fail to compile against this same
`qmk_firmware` checkout. This STM32 port uses the current `gpio_*` names;
worth knowing if the AtMega32u4 firmware ever needs a rebuild here too.

## Pin map (AtMega32u4 -> STM32F103)

| Function      | AtMega32u4 | STM32F103 (this PCB) | Net label   |
|---------------|------------|-----------------------|-------------|
| Matrix col 0  | B4         | B1                    | CL_1        |
| Matrix col 1  | B5         | B0                    | CL_2        |
| Matrix col 2  | B6         | A2                    | CL_3        |
| Matrix col 3  | B7         | A1                    | CL_4        |
| Matrix col 4  | C6         | A0                    | CL_5        |
| Matrix row 0  | C7         | A6                    | RW_1        |
| Matrix row 1  | D0         | A5                    | RW_2        |
| Matrix row 2  | D1         | A4                    | RW_3        |
| Matrix row 3  | D2         | A3                    | RW_4        |
| Encoder 1 A   | D3         | B9                    | ENC_A_1     |
| Encoder 1 B   | D4         | C14                   | ENC_B_1     |
| Encoder 1 SW  | D6         | C15                   | ENC_SW_1    |
| Encoder 2 A   | D7         | B7                    | ENC_A_2     |
| Encoder 2 B   | E6         | B8                    | ENC_B_2     |
| LED 1 (base)  | F1         | B15                   | Mode_1      |
| LED 2 (lay 1) | F4         | B14                   | Mode_2      |
| LED 3 (lay 2) | F5         | B13                   | Mode_3      |
| LED 4 (lay 3) | F6         | B12                   | Mode_4      |

Diode direction: COL2ROW on both boards (verified against the diode
orientation in `Switch.kicad_sch`).

Matrix column/row order (CL_1→col0 ... CL_5→col4, RW_1→row0 ... RW_4→row3)
is assumed from the net numbering since the physical silkscreen wasn't
available for cross-check. If the ported keymap comes out mirrored
left-right or top-to-bottom on the real board, it's this ordering that
needs flipping in `keyboard.json`, not the keymap itself.

## Hardware differences found during the port (not fixable in firmware)

The AtMega32u4 board has two inputs that this PCB does not route to the
MCU. Both are simply omitted here rather than faked:

- **Spacebar.** AtMega32u4 firmware reads a dedicated spacebar button via
  direct GPIO (F7), outside the 20-key matrix. `Switch.kicad_sch` on this
  PCB only has the 20 matrix switches (SW5-SW24) - there is no 21st
  direct-wired switch. If this PCB is meant to have a spacebar key, it
  needs a schematic/PCB revision; this firmware can't add a key where no
  switch exists.
- **Encoder 2 switch.** AtMega32u4 reads encoder 2's push switch via F0.
  On this schematic, encoder 2 (SW3) pin S1 is explicitly marked
  no-connect - only encoder 1's switch (ENC_SW_1 / PC15) is wired. Encoder
  2 still works for rotation (volume/mute), just not its click.

## What's and isn't VIA-programmable

- **Encoder 1's switch (click) is never VIA-programmable, on any layer.**
  It's read via direct GPIO in `matrix_scan_user()`, bypassing VIA's
  dynamic keymap entirely - the base-layer click opens the Innverse VIA
  configurator (`https://www.innverse.in/via-configurator`), and the other
  three layers run their own hardcoded sequences - Layer 2: screenshot
  (area/region select), Layer 3: open terminal, Layer 4: lock screen. None
  of this shows up in VIA, by design. This isn't a config limitation that could
  be lifted - VIA's encoder configuration (`encoder_map[]` +
  `ENCODER_MAP_ENABLE`) only ever exposes rotation (CW/CCW), never a click;
  making the click itself VIA-editable would require wiring it into the
  real key matrix (as a "direct pin" alongside the diode matrix, or an
  extra matrix position), which this PCB's encoder switch currently isn't.
- **All four of those hardcoded sequences are host-OS-aware**
  (`OS_DETECTION_ENABLE`, `rules.mk`): each branches on
  `detected_host_os()` and sends the right combo for Windows, macOS, or
  Linux - e.g. the base-layer click uses Win+R on Windows, Cmd+Space
  (Spotlight) on macOS, and Alt+F2 + `xdg-open <url>` on Linux. OS
  detection is a best-guess heuristic based on USB enumeration quirks
  (QMK's own docs: "shouldn't be relied on for critical functionality" -
  it can misread Linux as macOS, especially behind hubs/KVMs), so
  `OS_UNSURE` and anything unexpected fall back to the Windows sequence.

  | Layer | Action | Windows | macOS | Linux |
  |---|---|---|---|---|
  | 1 (base) | Open configurator | Win+R -> type URL -> Enter | Cmd+Space -> type URL -> Enter | Alt+F2 -> `xdg-open <url>` -> Enter |
  | 2 (`_LAY_1`) | Screenshot | Win+Shift+S | Cmd+Shift+4 | Shift+PrtScn (GNOME's area-select binding) |
  | 3 (`_LAY_2`) | Open terminal | Win+R -> type `cmd` -> Enter | Cmd+Space -> type `Terminal` -> Enter | Ctrl+Alt+T (direct binding, no launcher needed) |
  | 4 (`_LAY_3`) | Lock screen | Win+L | Ctrl+Cmd+Q | Win+L (shares Windows' GUI+L binding) |

  Windows types `cmd` rather than `wt` for the terminal, since Command
  Prompt is guaranteed present even without Windows Terminal installed.
  Linux's terminal shortcut is a real keybinding rather than a
  launcher-plus-typing dance, so it has no settle delay before it.
- **Encoder rotation is normally VIA-programmable** (that's what
  `encoder_map[]` + `ENCODER_MAP_ENABLE` give you) - **except** Layer 2
  (`_LAY_1`) encoder 1, which is hardcoded to backlight brightness (see
  below). VIA's UI will still show that one cell as an editable keycode
  (VIA has no "locked cell" concept), but whatever you assign there is
  silently ignored - `process_record_user()` in
  `keymaps/via_encoder/keymap.c` intercepts the encoder event and returns
  `false` before VIA's assigned keycode is acted on. Every other
  layer/encoder combination behaves normally.

  This is **not** done via `encoder_update_user()`, despite that looking
  like the obvious hook - with `ENCODER_MAP_ENABLE` on (required for VIA to
  expose rotation at all), QMK's encoder task
  (`quantum/encoder.c:encoder_handle_queue()`) routes rotation straight
  through `action_exec()`/`process_record()` using the dynamic keymap, and
  never calls `encoder_update_kb()`/`encoder_update_user()` - that hook is
  simply dead code whenever `ENCODER_MAP_ENABLE` is set. First attempt at
  this used `encoder_update_user()` and silently did nothing on real
  hardware (brightness never changed, volume kept working) until this was
  traced down to that QMK behavior.

## Backlight (added beyond the AtMega32u4 port - new for this PCB)

- **PA8 (LED_Drive)** gates Q5, an N-channel MOSFET low-side switch for the
  `GND-M` return rail shared by all 20 per-switch backlight LEDs (each wired
  `+5V -> 1k -> LED -> GND-M`). Active HIGH - same convention as the Mode
  LEDs.
- Driven by QMK's built-in PWM backlight feature rather than raw GPIO: PA8
  is TIM1_CH1 on the STM32F103, so `BACKLIGHT_ENABLE = yes` (`rules.mk`) +
  `BACKLIGHT_PIN`/`BACKLIGHT_PWM_DRIVER`/`BACKLIGHT_PWM_CHANNEL` (`config.h`)
  + `STM32_PWM_USE_TIM1` (`mcuconf.h`) + `HAL_USE_PWM` (`halconf.h`) give a
  real dimmable backlight instead of on/off. Defaults to full brightness on
  a fresh board (QMK's own stock EEPROM default), then persists whatever
  level you leave it at.
- **Brightness control is hardcoded to Layer 2 (`_LAY_1`) encoder 1
  rotation**, via `process_record_user()` in
  `keymaps/via_encoder/keymap.c`. This intentionally bypasses
  `encoder_map[]`/VIA for that one layer+encoder combination - see
  "What's and isn't VIA-programmable" above for why.

## Not used by this firmware (exists on the PCB, available later)

- **PC13** - green status LED (D6), driven directly through a 1k resistor,
  active-low (same convention as the LED in `QMK_3_keys_STM32`). Not
  referenced by the AtMega32u4 source, so left untouched here.

## Build

Copy this into `qmk_firmware/keyboards/` under its own name - **not**
`q16p_stm32`, which `qmk_firmware/keyboards/` already has (that's the
earlier `QMK_3_keys_STM32` 3-button prototype; copying on top of it would
overwrite it):

```bash
cp -r Q16P-Firmware-STM32 ~/qmk_firmware/keyboards/q16p_stm32_macropad
cd ~/qmk_firmware
qmk compile -kb q16p_stm32_macropad -km via_encoder
```

Verified in this repo's `/root/qmk_firmware` checkout: both `default` and
`via_encoder` keymaps compile clean (`via_encoder` -> 32782 bytes, well
under the 64KB flash budget).

## Flash

This board is bare silicon with no bootloader - firmware is flashed
directly over SWD at 0x08000000, the actual start of flash (see
`ld/STM32F103xB_no_boot.ld`, which maps the full 64K from 0x08000000 rather
than assuming a bootloader-reserved offset). `rules.mk` sets
`BOOTLOADER = custom`, which is QMK's mechanism for boards with no
bootloader to jump to - `platforms/chibios/bootloaders/custom.c` already
provides no-op stubs for `bootloader_jump()`/`mcu_reset()` so the build
doesn't need a real one. Same setup as the already-tested USB bring-up
binary for this PCB:

```bash
st-flash write .build/q16p_stm32_macropad_via_encoder.bin 0x08000000
```
