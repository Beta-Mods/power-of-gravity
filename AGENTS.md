# AGENTS.md — Moon Scaler NG

## Repository safety
- Do not destroy or overwrite pre-existing user work. Preserve unrelated changes.
- Never run destructive Git commands (`git reset --hard`, `git clean -fdx`, force-push, etc.).
- Never delete the user’s original version 1 Custom Moon Sizes files.
- Never push, publish, upload, or modify the Nexus page.
- Do not modify the normal Skyrim `Data` directory when a mod-manager staging folder is available.
- Use a separate development/test mod/profile. Do not overwrite an existing user-edited `MoonScalerNG.ini` during deployment.
- Do not package SKSE, Address Library, or SKSE Menu Framework with this mod.

## Project facts
- Product: **Moon Scaler NG** `1.0.0` — native SKSE/CommonLibSSE-NG plugin for Skyrim SE/AE (x64).
- Output: `Data/SKSE/Plugins/MoonScalerNG.dll` + `MoonScalerNG.ini`.
- Runtime requirements: SKSE64, Address Library, SKSE Menu Framework 3.x for the in-game page (INI still loads without it).
- Scope: no ESP/ESL/ESM, no Papyrus, no moon textures/meshes, no save dependency, no telemetry/network at runtime.
- Skyrim VR is disabled unless explicitly built and tested.

## Architecture
- `src/Plugin.cpp`: SKSE load, logging, messaging (`kPostLoad` installs hooks/UI, `kDataLoaded` captures GMST baseline, `kPostLoadGame`/`kNewGame` reapply active targets).
- `src/Hooks.cpp`: installs `RE::Moon::Init` (0x02) and `RE::Moon::Update` (0x03) vtable hooks via CommonLib/Address Library; calls original first, chains once.
- `src/MoonController.cpp`: captures load-order baseline from `iMasserSize`/`iSecundaSize`, applies active targets via SKSE task thread, applies absolute non-compounding local node scale to moon/shadow nodes after `Moon::Update`. Captures native size/node scale per instance and recaptures on node replacement; refreshes world transforms on scale changes.
- `src/Config.cpp`: INI load/save with defensive parsing, clamping, and temp-file replace (no in-place truncation). Path: `Data/SKSE/Plugins/MoonScalerNG.ini`.
- `src/SizeMath.h`: relative/absolute conversions, linked behavior helpers, bounds.
- `src/UI.cpp` + `extern/SKSEMenuFrameworkLite.h`: SKSE Menu Framework 3 C ABI consumer; registers page at `kPostLoad` only when framework is present/supported.
- `tests/`: portable size-math and runtime-controller tests with API-shaped stubs (do not prove real Skyrim ABI/runtime).

## Build / validation
- Windows release (VS 2022 x64, vcpkg manifest, pinned CommonLibSSE-NG `v15.7.3`):
  - Run from a VS 2022 x64 developer environment (or ensure vcpkg can validate the VS instance). Set `VCPKG_ROOT` to a bootstrapped vcpkg checkout.
  - `cmake --preset vs2022`
  - `cmake --build --preset vs2022-release --parallel`
  - `ctest --preset vs2022-release --output-on-failure`
  - `cmake --build .\build\vs2022 --config Release --target package`
  - Or: `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build-release.ps1 -Clean`
- Portable tests (no CommonLib/Skyrim): `./tools/validate-portable.sh` or `cmake -S . -B build/portable -DCMS2_BUILD_PLUGIN=OFF -DCMS2_BUILD_TESTS=ON && cmake --build build/portable && ctest --test-dir build/portable --output-on-failure`.

## Change discipline
- Verify hook signatures/vtable indices against the pinned CommonLibSSE-NG source before changing hooks.
- Never use hard-coded Skyrim executable addresses.
- Keep game/settings/scene mutations on the appropriate game/task thread; UI callbacks only update config/request tasks.
- Avoid per-frame logging and per-frame INI writes.
- Add/adjust regression tests for logic bugs found. Keep tests Release-safe (no `assert` that compiles out).
- Update docs (`README`, `BUILDING`, `VALIDATION`, `CHANGELOG`, notices/manifest) to match actual, verified results only.

## Release completion criteria
A release is complete only when the real MSVC x64 Release DLL builds, all CTest tests pass, artifacts are verified, SKSE loads the plugin, hooks/menu/settings behavior is validated as available on this PC, logs are clean, and documentation states only observed/automated facts. Do not claim in-game visual success without human observation or direct supplied evidence.