# Auto Swingshot for OpenRAC RAC1

**Experimental v0.1.3 — not yet verified in-game.**

A standalone quality-of-life mod that experiments with equipping the Swingshot automatically when Circle is pressed near a Versa-Target in Ratchet & Clank 1.

## Status and safety

**Auto-equip is enabled by default** as of v0.1.1. Pressing Circle near a detected target attempts to select the Swingshot, fire it and restore the previous weapon. If you prefer to check detection without changing gameplay, choose **Observe targets only** in the mod settings and restart.

**Auto-equip** mode attempts to switch to the Swingshot, use it, and restore the previous weapon. It is untested and may incorrectly select targets, eat a Circle press, or interfere with gameplay. Make a backup save before enabling it.

The detector recognises target object classes **758** and **803**, using straight-line distance, height difference and **Ratchet's facing angle**. Version 0.1.2 added a configurable facing cone (now **30° either side by default** in v0.1.3), so targets behind or far to the side no longer trigger a weapon switch. If Ratchet turns away before the Swingshot equips, the pending shot is cancelled. It still relies on the game's normal targeting, collision and attachment behaviour once the tool fires, and is not a substitute for the game's full target selector. Further tests are needed on both PAL and NTSC.

**Important for upgrades:** If you already set the facing angle to 60° in the launcher, the saved preference may remain at 60° after updating. Change **Facing angle (degrees)** to **30** in the mod settings and restart to test the narrower cone. You can adjust it to suit your preference.

## Download automatic builds

Every push to the `main` branch triggers the GitHub Actions **build** workflow. On a successful run, open **Actions → build → Artifacts**, then download `mods` for the merged cross-platform package or a platform-specific archive.

The workflow builds for Windows x64, Linux x64/ARM64, macOS Intel/Apple Silicon and Android ARM64 using the OpenRAC SDK pinned in `sdk.lock`. GitHub Actions reports any build or test failures. Builds are experimental and should not be presented as stable releases.

A version tag such as `v0.1.0` triggers an automatic GitHub release. **Do not tag until the in-game tests pass.**

## Testing

1. Install a successful build through the OpenRAC launcher.
2. For a non-invasive check, set **Swingshot activation → Observe targets only** and restart. For the feature itself, leave the default **Auto-equip** enabled; back up your save first.
3. On Aridia, approach a Swingshot target with a different weapon equipped.
4. Press Circle from a variety of positions and inspect log lines beginning `Auto Swingshot:`.
5. Check `target_slot`, `class`, `distance`, `height_diff`, `in_range`, `facing_dot`, `facing`, and `eligible`. The angle check is geometric, not the full retail target selector. When you face away from a nearby target, `facing=no` and `eligible=no` should be logged; your gun should fire normally.
6. In **Auto-equip** mode, test tapping and holding Circle; deliberately fire near a target while facing away; rotate away immediately after tapping Circle; verify ordinary gunfire isn't replaced; then test swinging between targets, pulling targets, restoring your weapon, death, travel and manual weapon selection.
7. Share the logs and any crashes before treating the feature as ready.

## Local development

Requires Python 3, CMake, Ninja, a C++17 compiler and an internet connection for the pinned SDK:

```sh
python tools/fetch_sdk.py
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

For detection-only tests without downloading the SDK:

```sh
cmake -S . -B build-test -DBUILD_OPENRAC_MOD=OFF
cmake --build build-test
ctest --test-dir build-test --output-on-failure
```

## Technical

The mod hooks `rac1.player.update`, reads player poses and live world objects through the OpenRAC SDK, and uses the RAC1 item request in experimental mode. No original game files are modified.

Source references: [OpenRAC SDK](https://github.com/OpenRAC/OpenRAC), [RAC1 weapon notes](https://github.com/OpenRAC/OpenRAC/blob/develop/docs/port/WEAPONS.md), [rac-rs auto gadget implementation](https://github.com/AsobiCodes/rac-rs/blob/main/rac-rs/src/rac1/retail/auto_gadget.rs).

Copyright (c) 2026 glawks. MIT License.
