# Moon Scaler NG

A native SKSE plugin that changes the apparent size of **Masser** and **Secunda** while Skyrim is running. This repository is the full source implementation for version 2 of [Custom Moon Sizes](https://www.nexusmods.com/skyrimspecialedition/mods/41327).

## What version 2 changes

Version 1 used separate ESP/ESL combinations for a small set of presets. Version 2 replaces those combinations with continuous in-game controls:

- Independent Masser and Secunda sliders in SKSE Menu Framework.
- Relative mode that scales the effective values produced by the user's load order.
- Absolute mode for exact `iMasserSize` and `iSecundaSize` values.
- Optional linked scaling, live preview, reset controls, and an expanded extreme range.
- Persistent INI configuration.
- No ESP/ESL and no Papyrus scripts.
- One Address Library-based DLL for Skyrim SE and AE.

## Project status

Version 2.0.0 source, packaging, build automation, platform-independent tests, and the real Windows (MSVC x64) build are complete. The Release DLL, PDB, mod-manager ZIP, symbols ZIP, and source ZIP were produced and verified by clean extraction. CTest (SizeMath + RuntimeController) passes. Remaining work is deployment/log validation and human-observed in-game visual/compatibility checks; see [VALIDATION.md](VALIDATION.md) and [IN_GAME_TEST_REPORT.md](IN_GAME_TEST_REPORT.md).

## Runtime requirements

- Skyrim Script Extender (SKSE64).
- Address Library for SKSE Plugins.
- SKSE Menu Framework 3.x for the in-game page. The API from 3.23 was used during development.

Without Menu Framework, the DLL still reads `Data/SKSE/Plugins/MoonScalerNG.ini` when Skyrim starts, but there is no in-game editor.

SE and AE are enabled by default. VR remains disabled until the UI and moon scene graph are validated in Skyrim VR.

## Installation

Install the compiled release ZIP with a mod manager, or copy its `Data` directory into a mod staging folder. Disable every version 1 Custom Moon Sizes ESP first; otherwise those records alter the load-order baseline detected by version 2.

Open the SKSE Menu Framework Mod Control Panel, select **Moon Scaler NG**, and open **Moon Sizes**.

## Controls

### Relative mode

The sliders represent a percentage of the final loaded values. This is the recommended mode when a weather or sky mod intentionally changes moon size.

```text
Masser target = loaded Masser value × Masser percentage
Secunda target = loaded Secunda value × Secunda percentage
```

### Absolute mode

The sliders write exact integer targets. This is closest to the behavior of the version 1 plugins.

### Other options

- **Link moon sizes** applies the same percentage to both moons. In Absolute mode it preserves each moon's ratio to its loaded baseline.
- **Live preview** applies a slider edit immediately. When disabled, use **Apply selected sizes**.
- **Allow extreme slider range** expands percentages from 10–500% to 1–1000% and absolute values from 1–500 to 1–2000.
- **Reset to loaded values** returns both moons to the values detected after the load order was processed.

## How it works

At `kDataLoaded`, the plugin captures the effective `iMasserSize` and `iSecundaSize` settings. Relative mode uses those values as its compatibility baseline.

Skyrim's moon objects cache size information. The plugin hooks `RE::Moon::Init` and `RE::Moon::Update`, calls the previously installed/original function first, captures each moon's native node scale, and then applies an absolute ratio to the visible moon node and phase-shadow node. Because every assignment is based on the captured native scale, the result does not compound frame by frame.

Slider callbacks only update configuration state and queue work through the SKSE task interface. Game settings are therefore written on Skyrim's task thread rather than directly from the UI render callback.

## Configuration

```ini
[MoonSizes]
Mode=Relative
MasserPercent=100
SecundaPercent=100
MasserAbsolute=90
SecundaAbsolute=40
LinkMoonSizes=false
LivePreview=true
AdvancedRange=false
```

`Mode` accepts `Relative` or `Absolute`. Percentage values are defensively clamped to 1–1000 and absolute values to 1–2000.

## Building

On Windows with Visual Studio 2022 and vcpkg configured:

```powershell
./tools/build-release.ps1
```

The script configures, builds, tests, and packages the mod. The resulting files are placed under:

```text
build/vs2022/dist/
build/vs2022/MoonScalerNG-2.0.0.zip
```

Detailed setup is in [BUILDING.md](BUILDING.md). A dependency-free portable test path is also available:

```bash
./tools/validate-portable.sh
```

## Diagnostics

The plugin writes `Moon Scaler NG.log` to the normal SKSE log directory under `Documents/My Games/Skyrim Special Edition/SKSE`.

Useful log events include:

- plugin and runtime version;
- hook installation;
- Menu Framework registration and detected framework version;
- captured Masser/Secunda load-order baseline;
- missing game settings or SKSE interfaces.

## Compatibility expectations

Moon texture and glow replacers are expected to remain compatible because this project replaces no moon assets. Relative mode is intended for weather and sky overhauls that edit the two size settings.

Another DLL that replaces the same `RE::Moon` virtual-table entries requires explicit testing. This plugin preserves a normal hook chain by calling the function pointer that was present when each hook was installed, but a later hook can still overwrite the final transform depending on its implementation and plugin load order.

Extremely large values can make the moons overlap or expose texture, mesh, culling, ENB, or shader limitations.

## Repository layout

```text
src/                  SKSE plugin implementation
extern/               Minimal Menu Framework ABI consumer
package/              Files copied into the release archive
cmake/                Windows version-resource template
tests/                Portable size-math and controller tests with API-shaped stubs
tools/                Release and validation scripts
.github/workflows/    Portable and Windows CI builds
```

## License

GPL-3.0-or-later. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for dependency notices.
