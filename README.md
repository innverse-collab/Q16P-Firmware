# Macropad

A custom 4x5 mechanical macropad with dual rotary encoders, built using QMK firmware. This macropad features a calculator-style layout with multiple layers for different functions.

## Features

- **4x5 Key Matrix**: 20 programmable keys arranged in a 4x5 grid
- **Dual Rotary Encoders**: Two encoders with click functionality and separate switch buttons
- **Multi-layer Support**: 4 different layers for various functions
- **LED Indicators**: 4 LEDs to indicate active layer
- **VIA Support**: Compatible with VIA for easy keymap configuration
- **Spacebar**: Dedicated spacebar key (handled via direct GPIO)
- **Dynamic Macros**: Support for up to 128 dynamic macros

## Hardware Specifications

- **Controller**: Pro Micro compatible
- **Matrix**: 4 rows × 5 columns (COL2ROW diode direction)
- **Encoders**: 2 rotary encoders with 4-step resolution
- **Encoder Switches**: 2 separate push buttons (one per encoder)
- **LEDs**: 4 status LEDs for layer indication
- **USB**: USB-C connector

### Pin Assignments

#### Matrix Pins
- **Columns**: B4, B5, B6, B7, C6
- **Rows**: C7, D0, D1, D2

#### Encoder Pins
- **Encoder 1**: D3 (A), D4 (B), D6 (Switch)
- **Encoder 2**: D7 (A), E6 (B), F0 (Switch)

#### LED Pins
- **LED 1**: F1 (Base layer indicator)
- **LED 2**: F4 (Layer 1 indicator)
- **LED 3**: F5 (Layer 2 indicator)
- **LED 4**: F6 (Layer 3 indicator)

#### Special Pins
- **Spacebar**: F7 (Direct GPIO with pulldown resistor)

## Keymaps

### Default Keymap
The default keymap provides a basic calculator layout with number keys 1-9.

### Via Encoder Keymap (Recommended)
The via_encoder keymap includes 4 layers with comprehensive functionality:

### Encoder Functions
- **Encoder 1**: Volume down/up (all layers)
- **Encoder 2**: Mute/F7 (all layers)
- **Encoder 1 Switch**: Enter (separate button on encoder 1)

## Building and Flashing

### Prerequisites
- [QMK Toolbox](https://github.com/qmk/qmk_toolbox) or [QMK CLI](https://docs.qmk.fm/#/newbs_getting_started)
- Compatible microcontroller (Pro Micro or similar)

### Build Instructions

1. Clone the QMK firmware repository:
```bash
git clone https://github.com/qmk/qmk_firmware.git
cd qmk_firmware
```

2. Copy this keymap to the QMK directory:
```bash
cp -r /path/to/this/project /path/to/qmk_firmware/keyboards/manan_5x4_v1/
```

3. Build the firmware:
```bash
qmk compile -kb manan_5x4_v1 -km via_encoder
```

4. Flash the firmware using QMK Toolbox or CLI:
```bash
qmk flash -kb manan_5x4_v1 -km via_encoder
```

### VIA Configuration

1. Install [VIA](https://caniusevia.com/)
2. Load the `via.json` file in VIA
3. Configure your keymap through the VIA interface
