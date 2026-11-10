# Power of Gravity — Design Plan

## Concept
A spell that "pulls the moon toward you." When cast, the moon grows larger (appears closer)
and the screen shakes. The moon returns to normal after the spell duration ends.

## Phases
1. **Cast** — player casts "Pull Moon" spell
2. **Phase 1 (0-3s)** — moon starts growing, screen begins shaking
3. **Phase 2 (3-8s)** — moon continues growing to max size, screen shakes harder
4. **Phase 3 (8-10s)** — moon holds at max size, screen shake fades
5. **End (10s)** — moon returns to normal size, screen stops shaking

## Architecture

### SKSE Plugin (PowerOfGravity.dll)
- Hooks RE::Moon::Update (same as Moon Scaler NG)
- Background thread applies scaling at safe point (proven pattern)
- Screen shake via camera node manipulation or RE::PlayerCamera
- Console command: `pullmoon` (triggers the effect for testing)
- Global state: `g_moonScale` (float), `g_shakeIntensity` (float), `g_phase` (enum)

### Spell Creation (requires Creation Kit or manual ESP)
- Spell: "Pull Moon" (Alteration or Illusion school)
- Magic Effect: "Pull Moon Effect" with duration (e.g., 10 seconds)
- Script on MGEF: calls SKSE native function to trigger scaling
- Cost: Magicka (configurable)

### Without Creation Kit
- Use console command `pullmoon` to trigger
- Or bind to a hotkey via SKSE
- Core functionality (scaling + shake) works without ESP

## Files to Create
1. src/Plugin.cpp — SKSE load, logging, background thread
2. src/Hooks.cpp — Moon::Init/Update hooks
3. src/MoonScaler.cpp — moon scaling logic (reuse Moon Scaler NG pattern)
4. src/ScreenShake.cpp — camera shake effect
5. src/Config.h/cpp — INI config (scale amount, duration, shake intensity)
6. src/UI.cpp — SKSE Menu Framework page (optional settings)
7. CMakeLists.txt — build config
8. MSNG_Readme.txt — plain text readme

## Screen Shake Approach
Option A: Manipulate camera node position (RE::PlayerCamera->cameraNode->local.translate)
Option B: Use RE::GameSetting or Papyrus Game.ShakeCamera (requires Papyrus)
Option C: Use RE::NiCamera shake (if available)

Recommendation: Option A — directly offset the camera node's position each frame
with a decaying sine wave, similar to how moon scaling works.

## Scaling Approach
- Use the proven Moon Scaler NG pattern (background thread + mesh scaling)
- Scale factor: 1.0 (normal) → targetScale (e.g., 3.0) over phase 1-2
- Decay back to 1.0 during phase 3
- Both Masser and Secunda scale together (linked)

## INI Config
[PowerOfGravity]
ScaleAmount=3.0
Duration=10.0
ShakeIntensity=0.5
ShakeFrequency=8.0
LivePreview=false

## ESP Creation (manual or Creation Kit)
If using Creation Kit:
1. Create new spell "Pull Moon" (SPEL)
2. Create magic effect "Pull Moon Effect" (MGEF)
3. Set MGEF script to call SKSE function
4. Set duration, cost, school
5. Package ESP with the mod

## Build
- Copy Moon Scaler NG project as base
- Rename all references
- Add screen shake code
- Add console command registration
- Build with MSVC 2022 + vcpkg
