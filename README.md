# Auto Swingshot for OpenRAC RAC1

**Experimental v0.1.0 — not yet verified in-game.**

A standalone quality-of-life mod that experiments with equipping the Swingshot automatically when Circle is pressed near a Versa-Target in Ratchet & Clank 1.

## Status and safety

The default **Observe targets** mode only logs candidate targets when Circle is pressed and does **not** change gameplay. This is the recommended first test.

**Experimental auto-equip** mode attempts to switch to the Swingshot, use it, and restore the previous weapon. It is untested and may incorrectly select targets, eat a Circle press, or interfere with gameplay. Make a backup save before enabling it.

The first detector recognises target object classes **758** and **803**, using straight-line distance and height difference. It does not yet perform the original game's line-of-sight, facing, or valid-target calculations. The weapon switch and input injection require further testing on both PAL and NTSC.

## Download automatic builds

Every push to the `main` branch triggers the GitHub Actions **build** workflow. On a successful run, open **Actions → build → Artifacts**, then download `mods` for the merged cross-platform package or a platform-specific archive.

The workflow builds for Windows x64, Linux x64/ARM64, macOS Intel/Apple Silicon and Android ARM64 using the OpenRAC SDK pinned in `sdk.lock`. GitHub Actions reports any build or test failures. Builds are experimental and should not be presented as stable releases.

A version tag such as `v0.1.0` triggers an automatic GitHub release. **Do not tag until the in-game tests pass.**

## Testing

1. Install a successful build through the OpenRAC launcher.
2. Leave the mode on **Observe targets**.
3. On Aridia, approach a Swingshot target with a different weapon equipped.
4. Press Circle from a variety of positions and inspect log lines beginning `Auto Swingshot:`.
5. Check `target_slot`, `class`, `distance`, `height_diff` and `in_range`. The logged distance is not a validated retail target check.
6. Only after observation works, back up a save, change to **Experimental auto-equip**, restart, and test tapping and holding Circle, returning to the old weapon, death, travel, and manual weapon selection.
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
