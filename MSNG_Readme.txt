Power of Gravity v1.0.0
=======================

A native SKSE plugin for Skyrim Special Edition / Anniversary Edition that
makes the sun grow larger (appearing to pull toward you) when activated,
with screen shake. Triggered via SKSE Menu Framework or console command.

No ESP, no Papyrus scripts, no texture or mesh replacements, no save dependency.


REQUIREMENTS
-----------
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.x (optional - for the in-game page)


INSTALLATION
-----------
1. Install with your mod manager (MO2 recommended).
2. Extract to SKSE\Plugins\ at root.
3. Enable the mod and launch via SKSE.


CONFIGURATION
-------------
INI path: Data/SKSE/Plugins/PowerOfGravity.ini

[PowerOfGravity]
ScaleAmount=3.0
Duration=10.0
ShakeIntensity=0.5
ShakeFrequency=8.0
LivePreview=false

ScaleAmount: how much the sun grows (1.0 = no change, 3.0 = 3x)
Duration: how long the effect lasts in seconds
ShakeIntensity: screen shake strength (0.0 = none, 2.0 = strong)
ShakeFrequency: screen shake speed


USAGE
-----
- Open SKSE Menu Framework -> Power of Gravity -> click "Pull Moon" or "Pull Sun"
- Or type "pullmoon" / "pullsun" in the console (~)
- The target grows over 3 seconds, holds for 5 seconds, then shrinks back
- Screen shakes during the grow and hold phases


HOW IT WORKS
------------
The plugin hooks RE::Sun::Update. A background thread applies scaling at a safe
point in the frame (matching the Moon_Illusion pattern). It scales the sun base
node, sun glare node, their meshes, and the glareScale value. Screen shake is
applied via camera node offset with a decaying sine wave.


COMPATIBILITY
-------------
Compatible with all mods except those that also modify sun scale. Works with
weather and sky mods. No ESP/ESL, so it does not count toward the plugin limit.

Extremely large values can expose rendering limitations.


LICENSE
-------
GPL-3.0-or-later.


THIRD-PARTY NOTICES
-------------------
CommonLibSSE-NG
  Builds and statically links CommonLibSSE-NG (GPL-3.0-or-later).

SKSE Menu Framework API
  extern/SKSEMenuFrameworkLite.h is a small independently written runtime loader.

Skyrim Script Extender and Address Library
  SKSE64 and Address Library are runtime requirements.


CHANGELOG
---------
2.0.0
  - Initial release as a native SKSE/CommonLibSSE-NG plugin.
  - Sun scaling with screen shake effect.
  - Background thread safe-point scaling.


SOURCE
------
Source code is included in the source zip. Build with Visual Studio 2022 and
vcpkg (see CMakeLists.txt).

The plugin writes Power of Gravity.log to the normal SKSE log directory under
Documents/My Games/Skyrim Special Edition/SKSE.