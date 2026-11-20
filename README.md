# ZMK config for beekeeb Toucan2 Keyboard

[The beekeeb Toucan2 Keyboard](https://beekeeb.com/introducing-toucan2/) is a wireless split 42-key column‑stagger keyboard that a display and a trackpad, with an aggressive stagger on the pinky columns.

# Customizations

- **Keymap & trackpad gestures**: [config/toucan.keymap](config/toucan.keymap)
  - Windows mode (`TOUCAN_WIN_MODE`) enabled; pinch zoom disabled.
  - Automouse activates Layer 7 (`AML`) while touching the trackpad (skipped on Layer 2 `CTC` and Layer 5 `SMB`).
  - Layer 2 (`CTC`): 1-finger 2D scroll; 2-finger swipe (once) sends `LG(TAB)` / `LG(RIGHT)` / `LG(TAB)` / `LG(LEFT)`.
  - Layer 4 (`SNP`): axis-constrained 1/5 cursor speed and halved 2-finger scroll.
  - Layer 5 (`SMB`): 1-finger horizontal swipe (once) sends `MB5` (left) / `MB4` (right); vertical swipe repeats `PG_DN` (up) / `PG_UP` (down).
- **General configs**: [boards/shields/toucan/toucan_left.conf](boards/shields/toucan/toucan_left.conf) and [boards/shields/toucan/toucan_right.conf](boards/shields/toucan/toucan_right.conf)
- **Swipe shortcuts**: the `swipe_button_mapper` node in [boards/shields/toucan/toucan.dtsi](boards/shields/toucan/toucan.dtsi)
- **Invert scroll / trackpad settings**: the `tps43_trackpad` node in [boards/shields/toucan/toucan_right.overlay](boards/shields/toucan/toucan_right.overlay)
- **Gesture-key processor tests**: `cc -std=c11 -Wall -Wextra -Werror tests/gesture_key_core_test.c -o /tmp/gesture_key_core_test && /tmp/gesture_key_core_test`

# License

The code in this repo is available under the MIT license.

The included shield nice_view_gem is modified from https://github.com/M165437/nice-view-gem licensed under the MIT License.

The linked trackpad module is based on https://github.com/geeksville/zmk_driver_azoteq

ZMK code snippets are taken from the ZMK documentation under the MIT license.

The embedded font QuinqueFive is designed by GGBotNet, licensed under under the SIL Open Font License, Version 1.1.
