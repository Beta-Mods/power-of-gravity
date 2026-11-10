# Power of Gravity

A native SKSE plugin for Skyrim Special Edition that makes the sun or moon grow larger (appearing to pull toward you) with screen shake and thunder sound. Hostile NPCs in range are lifted upward and fall back down when the effect ends.

## Features

- **Pull Moon** / **Pull Sun** triggers via SKSE Menu Framework buttons
- Sun and moon visual scaling with phased animation (grow → hold → shrink)
- Screen shake via game's built-in camera shake API
- Thunder sound on trigger
- Hostile NPCs lifted upward with accelerating speed
- All settings adjustable in-game via SKSE Menu Framework 3.x
- No ESP, no Papyrus scripts, no texture or mesh replacements
- Persistent INI configuration

## Requirements

- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.x (for in-game page)

## Installation

1. Install with your mod manager (MO2 recommended)
2. Extract to `SKSE\Plugins\` at root
3. Enable the mod and launch via SKSE

## Configuration

INI path: `Data/SKSE/Plugins/PowerOfGravity.ini`

```ini
[PowerOfGravity]
ScaleAmount=3.0
Duration=5.0
ShakeIntensity=0.5
Gravity=15.0
GravityRadius=5000.0
```

| Setting | Description |
|---------|-------------|
| ScaleAmount | How much the sun/moon grows (1.0 = no change, 3.0 = 3x) |
| Duration | How long the effect lasts in seconds |
| ShakeIntensity | Screen shake strength (0.0 = none, 2.0 = strong) |
| Gravity | NPC lift speed per frame (1-50) |
| GravityRadius | Radius in game units to affect NPCs |

## Building

Build with Visual Studio 2022 and vcpkg. See CMakeLists.txt for details.

```powershell
cmake --preset vs2022-release
cmake --build --preset vs2022-release --config Release
```

## License

GPL-3.0-or-later. See LICENSE for details.
