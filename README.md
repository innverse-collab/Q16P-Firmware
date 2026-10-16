# Q16P-Firmware (STM32)

QMK firmware for the Q16P 4x5 macropad, STM32F103C8 revision. Port of
[Q16P-Firmware-AtMega32u4](../Q16P-Firmware-AtMega32u4) to the
`Macropad_4x4_stm32` PCB, with the hardware adapted to the new MCU and a
few STM32-specific additions layered on top. Confirmed working on hardware
with QMK, VIA, and the `via_encoder` keymap's encoder shortcuts and
backlight control.

## Status

| Component | Status |
|---|---|
| Core matrix / keymap | Working, verified on hardware |
| VIA | Working, verified on hardware |
| Rotary encoders (rotation + click) | Working, verified on hardware |
| Per-switch PWM backlight | Working, verified on hardware |
| OS-aware encoder shortcuts | Verified on Linux; Windows/macOS compiled but not yet hardware-tested |

## Features

- 4x5 key matrix (20 keys), COL2ROW diode direction
- 2 rotary encoders; 1 has a click switch, both support rotation
- 4 VIA layers with a progressive LED indicator (N LEDs lit for layer N -
  see [Layer Indicator LEDs](#layer-indicator-leds))
- Per-switch PWM backlight (brightness adjustable from the keyboard itself)
- Encoder 1's click runs a hardcoded, OS-aware shortcut per layer (opens a
  URL, screenshot, terminal, lock screen) - see [Encoder Click
  Shortcuts](#encoder-click-shortcuts)
- One-time LED/backlight reveal animation on power-up - see [Boot
  Animation](#boot-animation)

## Hardware

Target: STM32F103C8T6 on the `Macropad_4x4_stm32` PCB. Pin assignments
below were read directly off the schematic (`Macropad_4x4_stm32.pdf`:
`MCU.kicad_sch` / `Switch.kicad_sch`), not assumed.

### Pin map (AtMega32u4 -> STM32F103)

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

Diode direction (COL2ROW) is the same on both boards, verified against the
diode orientation in `Switch.kicad_sch`.

> **Note:** matrix column/row order (CL_1→col0 ... CL_5→col4, RW_1→row0 ...
> RW_4→row3) is inferred from the net numbering, since the physical
> silkscreen wasn't available to cross-check. If the keymap comes out
> mirrored left-right or top-to-bottom on the real board, fix the ordering
> in `keyboard.json`, not the keymap.

### Layer Indicator LEDs

The 4 Mode LEDs show the active layer progressively - the LED count lit
equals the layer number, rather than lighting a single LED per layer:

| Layer | LED 1 | LED 2 | LED 3 | LED 4 |
|---|---|---|---|---|
| 1 (base) | on | off | off | off |
| 2 (`_LAY_1`) | on | on | off | off |
| 3 (`_LAY_2`) | on | on | on | off |
| 4 (`_LAY_3`) | on | on | on | on |

Implemented in `layer_state_set_user()` (`keymaps/via_encoder/keymap.c`).

### Boot Animation

On power-up, `keyboard_post_init_user()` runs a one-time ~1.8s animation
before settling into normal operation:

- The 4 Mode LEDs fade in individually, staggered (120ms apart, each with
  its own 400ms fade), so LED 1 is already brightening by the time LED 2
  starts - a smooth cascade rather than a 1/2/3/4 step sequence. They hold
  briefly at full brightness, then fade back out in reverse order
  (4→3→2→1), ending on whatever the actual boot layer calls for (always
  Layer 1/base on power-on, so LED 1 settles back on and the rest stay
  off).
- The per-switch backlight fades in and back out over the same window,
  peaking at its saved brightness level partway through and easing back
  down before settling there again at the end.
- Both use an ease-in-out (smoothstep) curve rather than a linear ramp, for
  the gentler acceleration/deceleration feel of the fade.

**Why this needed more than `backlight_level()` stepping:** the backlight
abstraction only has `BACKLIGHT_LEVELS` (8) discrete steps - cycling
through them, even quickly, reads as visible jumps rather than a continuous
glow. The animation instead drives the PWM channel's duty cycle directly
(`pwmEnableChannel`/`PWM_FRACTION_TO_WIDTH`, bypassing the level
abstraction entirely) for a genuinely continuous ramp, only handing control
back to the normal `backlight_set()` API once the animation ends.

The 4 Mode LEDs have no PWM hardware at all (plain GPIO), so their fade is
a software PWM: a fast on/off carrier (125Hz, 8 steps, driven off the same
1ms animation tick) whose duty cycle is continuously swept by the
per-LED envelope - fast enough to read as a smooth brightness change rather
than a flicker.

If the backlight was left off (brightness at 0), the fade step is skipped
entirely - only the LED cascade plays. The animation never touches EEPROM;
it computes the real saved level once up front, and the final
`backlight_set()` call restores that exact (perceptually-corrected) value,
so the saved brightness is unaffected by the animation itself.

This runs once, blocking, before the main scan loop starts (USB
enumeration is already underway by this point) - it adds about 1.8 seconds
to boot time and no ongoing CPU/flash cost afterward.

### Known hardware limitations

This PCB revision doesn't route two inputs that the AtMega32u4 board has;
both are simply omitted in firmware rather than faked:

- **No spacebar pad.** AtMega32u4 reads a dedicated spacebar switch via
  direct GPIO (F7), outside the 20-key matrix. `Switch.kicad_sch` on this
  PCB only has the 20 matrix switches (SW5-SW24) - there's no 21st
  direct-wired switch to read. Adding one needs a schematic/PCB revision.
- **No encoder 2 click.** AtMega32u4 reads encoder 2's push switch via F0.
  On this schematic, encoder 2 (SW3) pin S1 is explicitly marked
  no-connect. Encoder 2 still rotates fine (volume/mute); only its click is
  unavailable.

### Not used by this firmware

- **PC13** - green status LED (D6), active-low through a 1k resistor. Not
  referenced by the AtMega32u4 source, so left untouched here.

## Encoder Click Shortcuts

Encoder 1's click is hardcoded per layer (see [VIA
Programmability](#via-programmability) for why) and branches on
`detected_host_os()` (`OS_DETECTION_ENABLE`) to send the right shortcut for
Windows, macOS, or Linux:

| Layer | Action | Windows | macOS | Linux |
|---|---|---|---|---|
| 1 (base) | Open VIA configurator | Win+R → type URL → Enter | Cmd+Space → type URL → Enter | Alt+F2 → `xdg-open <url>` → Enter |
| 2 (`_LAY_1`) | Screenshot | Win+Shift+S | Cmd+Shift+4 | Shift+PrtScn (GNOME's area-select binding) |
| 3 (`_LAY_2`) | Open terminal | Win+R → type `cmd` → Enter | Cmd+Space → type `Terminal` → Enter | Ctrl+Alt+T (direct binding) |
| 4 (`_LAY_3`) | Lock screen | Win+L | Ctrl+Cmd+Q | Win+L (shares Windows' GUI+L binding) |

Notes:
- Windows types `cmd` rather than `wt` for the terminal, since Command
  Prompt is guaranteed present even without Windows Terminal installed.
- Linux's terminal shortcut is a direct keybinding, not a
  launcher-plus-typing sequence, so it has no settle delay before it.
- OS detection is a best-guess heuristic based on USB enumeration quirks
  (per QMK's own docs: "shouldn't be relied on for critical
  functionality" - it can misread Linux as macOS, especially behind hubs
  or KVMs). `OS_UNSURE` and anything unexpected fall back to the Windows
  sequence.

## VIA Programmability

- **Encoder 1's click is never VIA-programmable, on any layer.** It's read
  via direct GPIO in `matrix_scan_user()`, bypassing VIA's dynamic keymap
  entirely. This isn't a configuration choice that could be flipped - VIA's
  encoder support (`encoder_map[]` + `ENCODER_MAP_ENABLE`) only ever
  exposes rotation (CW/CCW), never a click. Making the click itself
  VIA-editable would require wiring it into the real key matrix (as a
  "direct pin" alongside the diode matrix, or an extra matrix position),
  which this PCB's encoder switch currently isn't.
- **Encoder rotation is normally VIA-programmable**, except Layer 2
  (`_LAY_1`) encoder 1, which is hardcoded to backlight brightness (see
  [Backlight](#backlight)). VIA's UI will still show that cell as editable
  (VIA has no "locked cell" concept), but whatever is assigned there is
  silently ignored: `process_record_user()` in `keymaps/via_encoder/keymap.c`
  intercepts the encoder event and returns `false` before VIA's resolved
  keycode is acted on. Every other layer/encoder combination behaves
  normally.

> **Implementation note:** this is done in `process_record_user()`, not
> `encoder_update_user()`, despite the latter looking like the obvious
> hook. With `ENCODER_MAP_ENABLE` on (required for VIA to expose rotation
> at all), QMK's encoder task (`quantum/encoder.c:encoder_handle_queue()`)
> routes rotation straight through `action_exec()`/`process_record()` using
> the dynamic keymap, and never calls
> `encoder_update_kb()`/`encoder_update_user()` - that hook is dead code
> whenever `ENCODER_MAP_ENABLE` is set.

## Backlight

- **PA8 (`LED_Drive`)** gates Q5, an N-channel MOSFET low-side switch for
  the `GND-M` return rail shared by all 20 per-switch backlight LEDs (each
  wired `+5V -> 1k -> LED -> GND-M`). Active HIGH, same convention as the
  layer-indicator LEDs.
- Driven by QMK's PWM backlight feature rather than raw GPIO, since PA8 is
  TIM1_CH1 on the STM32F103: `BACKLIGHT_ENABLE = yes` (`rules.mk`) +
  `BACKLIGHT_PIN`/`BACKLIGHT_PWM_DRIVER`/`BACKLIGHT_PWM_CHANNEL` (`config.h`)
  + `STM32_PWM_USE_TIM1` (`mcuconf.h`) + `HAL_USE_PWM` (`halconf.h`). This
  gives a real dimmable backlight instead of on/off. Defaults to full
  brightness on a fresh board (QMK's stock EEPROM default), then persists
  whatever level is left set.
- Brightness is adjusted by Layer 2 (`_LAY_1`) encoder 1 rotation,
  hardcoded via `process_record_user()` (see [VIA
  Programmability](#via-programmability)).

## Build

Copy into `qmk_firmware/keyboards/` under its own name:

```bash
cp -r Q16P-Firmware ~/qmk_firmware/keyboards/q16p_stm32_macropad
cd ~/qmk_firmware
qmk compile -kb q16p_stm32_macropad -km via_encoder
```

> Check `qmk_firmware/keyboards/` first and pick a directory name that
> isn't already in use there, so you don't overwrite another keyboard.

Verified against the `qmk_firmware` checkout at `/root/qmk_firmware`: both
`default` and `via_encoder` keymaps compile clean (`via_encoder` ->
34002 bytes, well under the 64KB flash budget).

One mechanical (non-behavioral) change was needed versus the AtMega32u4
source to get it compiling: the AtMega32u4 keymap calls QMK's old camelCase
GPIO API (`setPinOutput`/`writePinHigh`/`writePinLow`/`setPinInputHigh`/
`readPin`), renamed repo-wide in Feb 2024 (`docs/ChangeLog/20240225.md`) to
`gpio_set_pin_output`/`gpio_write_pin_high`/`gpio_write_pin_low`/
`gpio_set_pin_input_high`/`gpio_read_pin`. The old names no longer exist in
this `qmk_firmware` checkout at all (not even for AVR), so the AtMega32u4
source as currently written would also fail to compile here. This port
uses the current `gpio_*` names.

## Flash

This board is bare silicon with no bootloader - firmware is flashed
directly over SWD at 0x08000000, the real start of flash (see
`ld/STM32F103xB_no_boot.ld`, which maps the full 64K from 0x08000000 rather
than assuming a bootloader-reserved offset). `rules.mk` sets
`BOOTLOADER = custom`, QMK's mechanism for boards with no bootloader to
jump to - `platforms/chibios/bootloaders/custom.c` provides no-op stubs
for `bootloader_jump()`/`mcu_reset()` so the build doesn't need a real one.

```bash
st-flash write .build/q16p_stm32_macropad_via_encoder.bin 0x08000000
```

## Repository Layout

```
Q16P-Firmware/
├── keyboard.json           # Matrix, pins, USB IDs, encoder config
├── config.h                # Backlight pin/timer config
├── rules.mk                # Bootloader, linker script, feature flags
├── mcuconf.h / halconf.h   # ChibiOS clock/HAL config
├── ld/                     # Custom no-bootloader linker script
├── via.json                # VIA Configurator definition
├── build/                  # Compiled binaries, ready to flash
└── keymaps/
    ├── default/            # Minimal keymap, no VIA/encoder features
    └── via_encoder/        # Full keymap: VIA, encoders, backlight, OS-aware shortcuts
```
