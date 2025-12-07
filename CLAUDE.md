# CLAUDE.md - Open Wanzer Project Context

Last Updated: 2025-12-07

---

## Critical Constraints

### Compiler Warnings: Zero Tolerance
Build MUST be warning-free for our code. Fix immediately, never commit warnings.
```bash
make clean && make 2>&1 | grep "^src/" | grep "warning:"  # Must be empty
```

### Code Formatting: Mandatory
Run clang-format on all edited files before committing.
```bash
clang-format -i src/File.cpp include/File.hpp
```

### Directory Structure: Flat
ALL headers in `include/`, ALL sources in `src/` - no subdirectories.

### Include Order: Strict
```cpp
#include "ProjectHeaders.hpp"  // Project first, alphabetically
#include "Raylib.hpp"

#include <algorithm>           // Stdlib second, alphabetically  
#include <vector>
```

Raylib headers are in `include/`: `Raylib.hpp`, `Raygui.hpp`, `Raymath.hpp`, `Rlgl.hpp`

---

## Build Commands

```bash
make              # Build release
make run          # Build and run
make format       # Format all code
make clean        # Remove build artifacts
```

Binary: `build/openwanzer`

---

## Domain Knowledge

**Game Type:** Turn-based tactical mech combat (Panzer General + BattleTech)

**Hex System:** Axial coordinates internally, flat-top hexes

**Armor System (BattleTech-style):** 8 locations: Head, CT, LT, RT, LA, RA, LL, RL
- Each location tracks current/max armor independently
- Damage via hit tables based on attack arc (front/side/rear)
- Paperdoll UI shows visual damage

**Combat:** Attack arcs affect hit tables, range affects hit chance, hardness system for target types

---

## Session-End Protocol

Update this file's "Last Updated" date if you made significant architectural changes or added critical constraints.
