# Auto Swingshot

**Auto Swingshot** automatically equips the Swingshot when you press **Circle** near a Versa-Target in **Ratchet & Clank (2002)** on OpenRAC. After using it, your previous weapon is restored, without having to open the weapon wheel.

## Features

- Works with **swinging and pulling Versa-Targets**.
- Automatically equips and fires the Swingshot, then restores your previously selected weapon (including when you were using the wrench).
- Supports chaining between swinging targets while airborne, without switching weapons mid-swing.
- Checks target distance, height and facing angle to avoid unwanted activations.
- Leaves attachment, movement and release behaviour to the original game.

**How activation works:** Press Circle **again** when the target is within range. The mod detects a **fresh Circle press**; if you are already holding Circle to fire another weapon and then move into range, it will not unexpectedly interrupt your weapon fire.

## Settings

The default settings were tuned through actual NTSC-U gameplay testing; you can leave them unchanged for normal play.

| Setting | Default | What it controls |
| --- | --- | --- |
| Target radius | **16** | Straight-line distance to a Versa-Target, in game units |
| Height difference | **7** | Maximum vertical separation, in game units |
| Facing angle | **30°** | Allowed angle to either side of Ratchet's forward direction |
| Swingshot activation | **Auto-equip** | Optional **Observe targets only** mode logs candidate targets without changing weapons |

Changes require a game restart. **Existing settings are preserved when updating:** if you used an earlier test build, manually set the values to **16 / 7 / 30°** if you want the new defaults.

## Compatibility and limitations

- Supports **RAC1 NTSC-U and PAL**. **NTSC-U has been tested in-game; PAL has been built but is not yet gameplay-tested.**
- Native builds are provided for **Windows x64, Linux x64/ARM64, macOS Intel/Apple Silicon and Android ARM64**.
- Tested on NTSC-U with **Lock Strafe, Pause Weapon Wheel, Expanded Quick Select, Keyboard & Mouse, Ammo Max, Damage Numbers, Better Pause Menu and Xbox Icons** active simultaneously. Other mod combinations are not guaranteed.
- Range and facing checks determine when the mod attempts to equip; the **game itself decides whether the Swingshot connects**. Missed connections or detaching when falling too far can happen just as in the original game.
- Pressing Circle while aiming near a target can select the Swingshot instead of firing your current weapon. You can still fire normally outside the target's range or while facing away.
- If the target moves outside the permitted range or facing angle before equipment is ready, the pending shot is cancelled.

## Troubleshooting

Make sure you own the Swingshot, the mod is enabled, and you are within the configured range while facing the target. If you were already holding Circle, release it and press again.

For detailed diagnostics, check the game log for lines beginning `Auto Swingshot:`. The message `equipped; handing Circle to the game` confirms that input was passed to the game, **not** that the hook successfully connected.

Please report issues on [GitHub Issues](https://github.com/glawks/auto-swingshot/issues), including the disc version (NTSC-U/PAL), active mods and relevant log lines.

## For developers

The mod is built against the SDK pinned in [`sdk.lock`](sdk.lock). See [development and release instructions](docs/DEVELOPMENT.md) for local builds, tests and tagging.

Licensed under the [MIT License](LICENSE). Copyright © 2026 glawks.
