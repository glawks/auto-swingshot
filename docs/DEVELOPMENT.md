# Development and releasing

The main README is player-facing because OpenRAC displays it during mod installation. Technical notes live here.

## Local build

Requirements: Python 3, CMake 3.20+, Ninja, a C++17 compiler and internet access to fetch the pinned SDK.

```sh
python tools/fetch_sdk.py
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

For standalone geometry tests without the SDK:

```sh
cmake -S . -B build-test -DBUILD_OPENRAC_MOD=OFF
cmake --build build-test
ctest --test-dir build-test --output-on-failure
```

The mod hooks `rac1.player.update`, reads player and target positions through the SDK and requests item changes through RAC1 game memory. It does not modify the original game files. The target detector recognises object classes **758** (pulling) and **803** (swinging). Its geometry filter is not the game's complete retail connection check.

See the [OpenRAC mod documentation](https://github.com/OpenRAC/OpenRAC/blob/develop/docs/port/MODS.md) and [weapon notes](https://github.com/OpenRAC/OpenRAC/blob/develop/docs/port/WEAPONS.md).

## Release checklist

1. Update the same version in `openrac-mod.json` and `OPENRAC_MOD_CPP` in `src/mod.cpp`. Edit `RELEASE_NOTES.md` for that version.
2. Check the README, manifest metadata, settings and MIT license.
3. Push the release commit to `main`; wait for the **build** workflow to succeed on every platform and for **merge** to produce the combined archive.
4. Download the **mods** Actions artifact. Check that it has `auto-swingshot-<version>.zip`, the platform-specific archives and `SHA256SUMS`. If gameplay changed, smoke-test the new Windows DLL.
5. Tag the verified commit `v<version>` and push that tag. The workflow checks that the tag matches the manifest, publishes the archives and uses `RELEASE_NOTES.md` as the release description.
6. Check the GitHub Release and verify that **OpenRAC Launcher → Mods → Add from a link** installs the appropriate platform build.

**Do not move or reuse published tags:** the launcher checks archive hashes and may refuse changed assets.

## Regression scenarios

- New Circle press at an eligible target; auto-equips, fires and eventually restores the previous weapon.
- Circle press while facing away or outside range; ordinary weapon firing works.
- Holding Circle while entering target range does **not** trigger auto-equip.
- Turn away during equipment switching; pending shot cancels.
- Pulling Versa-Targets and chained airborne swings preserve the game's usual behaviour.
- Wrench handling restores the previously selected weapon.
- Test with other mods that hook `rac1.player.update`, especially Pause Weapon Wheel and Expanded Quick Select.
- PAL remains unverified in-game; successful compilation is not a gameplay compatibility guarantee.
