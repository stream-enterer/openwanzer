# Contributing to Open Wanzer

Thank you for your interest in contributing to Open Wanzer! This document provides guidelines and instructions for contributing to the project.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Setup](#development-setup)
3. [Coding Standards](#coding-standards)
4. [Project Structure](#project-structure)
5. [Making Changes](#making-changes)
6. [Submitting Changes](#submitting-changes)
7. [AI Assistant Guidelines](#ai-assistant-guidelines)

---

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- GNU Make
- Git
- Basic knowledge of C++ and game development

### First Steps

1. **Fork the repository** on GitHub
2. **Clone your fork**:
   ```bash
   git clone https://github.com/YOUR_USERNAME/openwanzer.git
   cd openwanzer
   ```
3. **Build the project**:
   ```bash
   make
   ```
4. **Run the game**:
   ```bash
   ./openwanzer
   ```

---

## Development Setup

### Build Commands

```bash
make              # Build release version
make debug        # Build with debug symbols
make clean        # Remove build artifacts
make run          # Build and run
make format       # Format code with clang-format
make help         # Show all targets
```

### IDE Setup

#### VS Code
- Install C/C++ extension
- Use the provided `.clang-format` for formatting
- Configure include paths: `./include`, `./src`

#### CLion
- Open project directory
- CLion should auto-detect the Makefile
- Set include directories in project settings

#### Vim/Neovim
- Use clangd for LSP
- Configure include paths in compile_commands.json

---

## Coding Standards

### File Naming

- **Source Files**: PascalCase (e.g., `GameState.cpp`)
- **Header Files**: PascalCase (e.g., `Unit.h`)
- **No Subdirectories**: All headers in `include/`, all source in `src/`

### Naming Conventions

```cpp
// Files
GameState.cpp, Unit.h

// Classes and Structs
class GameState { ... };
struct Unit { ... };

// Functions and Methods
void calculate_damage();
int get_unit_count();

// Variables
int unit_count;
float max_health;

// Constants
const int MAX_UNITS = 100;

// Enums
enum class TerrainType {
    PLAINS,
    FOREST,
    MOUNTAIN
};

// Namespaces
namespace GameLogic { ... }
```

### Header Guards

```cpp
#ifndef OPENWANZER_FILENAME_H
#define OPENWANZER_FILENAME_H

// Content

#endif // OPENWANZER_FILENAME_H
```

### Code Style

- **Indentation**: 2 spaces (no tabs)
- **Braces**: K&R style (opening brace on same line)
- **Line Length**: 100 characters maximum
- **Comments**: Use `//` for single-line, `/* */` for multi-line

**Example**:
```cpp
void process_unit(Unit* unit) {
  if (unit == nullptr) {
    return;
  }

  // Process unit logic here
  for (int i = 0; i < unit->weapon_count; i++) {
    process_weapon(&unit->weapons[i]);
  }
}
```

### Include Order

```cpp
// 1. Corresponding header (for .cpp files)
#include "Unit.h"

// 2. C system headers
#include <cstdlib>
#include <cmath>

// 3. C++ standard library headers
#include <string>
#include <vector>

// 4. Third-party library headers
#include "raylib.h"

// 5. Project headers
#include "GameState.h"
#include "Enums.h"
```

### Documentation

Use inline comments for complex logic:

```cpp
// Calculate Manhattan distance in axial coordinates
// This gives us the hex distance between two points
int distance = abs(a.q - b.q) + abs(a.r - b.r) + abs(a.s - b.s)) / 2;
```

For functions, use brief comments:

```cpp
// Returns true if the unit can move to the target hex
bool can_move_to(Unit* unit, HexCoord target);
```

---

## Project Structure

### Directory Layout

```
openwanzer/
├── src/                   # All .cpp source files
├── include/               # All .h header files
├── lib/                   # Third-party libraries
├── resources/             # Game assets
├── tests/                 # Unit tests (future)
├── docs/                  # Documentation
├── scripts/               # Build scripts
└── examples/              # Example scenarios
```

### Module Organization

See `docs/ARCHITECTURE.md` for detailed module structure.

**Quick Reference**:
- **Core**: GameState, Unit, basic types
- **Game Logic**: Combat, pathfinding, systems
- **Rendering**: Graphics and UI
- **Input**: User input and camera
- **Config**: Settings and persistence
- **UI**: Complex UI components

---

## Making Changes

### Workflow

1. **Create a branch**:
   ```bash
   git checkout -b feature/my-new-feature
   ```

2. **Make your changes**:
   - Follow coding standards
   - Keep changes focused
   - Test as you go

3. **Format your code**:
   ```bash
   make format
   ```

4. **Build and test**:
   ```bash
   make clean
   make
   ./openwanzer
   ```

5. **Commit your changes**:
   ```bash
   git add .
   git commit -m "Add feature: brief description"
   ```

### Commit Messages

Format: `Type: Brief description`

**Types**:
- `FEAT`: New feature
- `FIX`: Bug fix
- `REFACTOR`: Code restructuring
- `DOCS`: Documentation changes
- `STYLE`: Formatting, whitespace
- `TEST`: Adding tests
- `CHORE`: Build, dependencies

**Examples**:
```
FEAT: Add unit experience system
FIX: Correct pathfinding edge case
REFACTOR: Simplify combat calculation
DOCS: Update ARCHITECTURE.md with new module
```

### Testing

**Manual Testing Checklist**:
- [ ] Build completes without warnings
- [ ] Game launches successfully
- [ ] New feature works as expected
- [ ] Existing features still work
- [ ] No crashes or errors

**Future**: Automated unit tests in `tests/` directory

---

## Submitting Changes

### Pull Request Process

1. **Push to your fork**:
   ```bash
   git push origin feature/my-new-feature
   ```

2. **Create Pull Request** on GitHub

3. **PR Description** should include:
   - What changes were made
   - Why the changes were made
   - How to test the changes
   - Screenshots (if UI changes)

4. **Address Review Feedback**:
   - Make requested changes
   - Push updates to the same branch
   - Respond to comments

5. **PR Approval**:
   - At least one maintainer approval required
   - CI checks must pass (when implemented)
   - No merge conflicts

### PR Template

```markdown
## Description
Brief description of changes

## Motivation
Why these changes are needed

## Changes Made
- Added X
- Modified Y
- Removed Z

## Testing
How to test these changes

## Screenshots (if applicable)
[Add screenshots here]

## Checklist
- [ ] Code follows project style guidelines
- [ ] Code has been formatted with `make format`
- [ ] Documentation updated (if needed)
- [ ] Tested manually
- [ ] No new warnings or errors
```

---

## AI Assistant Guidelines

### Using Claude or Other AI Assistants

If using Claude Code or other AI assistants for development:

1. **Always read `CLAUDE.md` first** - This file contains project context

2. **Update `CLAUDE.md` at session end**:
   - Add new files created
   - Document architecture changes
   - Update "Recent Changes" section
   - Update "Last Updated" date

3. **Maintain consistency**:
   - Follow existing patterns
   - Match current code style
   - Respect module boundaries

4. **Verify AI-generated code**:
   - Always review before committing
   - Test thoroughly
   - Ensure it follows project standards

### CLAUDE.md Session-End Protocol

**CRITICAL**: At the end of every session with Claude:

```markdown
1. Scan codebase for changes
2. Update CLAUDE.md with:
   - New files created
   - Major refactorings
   - Architecture changes
   - Breaking changes
3. Update "Recent Changes" section
4. Update "Last Updated" date
```

This ensures continuity across development sessions.

---

## Specific Contribution Areas

### Adding New Unit Types

1. Add enum to `Enums.h`: `UnitClass`
2. Add unit data to `Unit.cpp`: `getUnitStats()`
3. Add unit sprite/rendering to `Rendering` module
4. Update hit tables if needed
5. Test with various scenarios

### Adding New Game Mechanics

1. Determine appropriate module (see `ARCHITECTURE.md`)
2. Add declarations to module header in `include/`
3. Implement in corresponding `.cpp` in `src/`
4. Update `GameState` if new state needed
5. Add UI elements if needed
6. Document in `ARCHITECTURE.md`

### Improving Graphics/UI

1. Locate rendering code in `Rendering` module
2. Make changes (follow raylib patterns)
3. Test at different resolutions
4. Test with different themes
5. Update screenshots in documentation

### Fixing Bugs

1. Create issue on GitHub describing bug
2. Reference issue in commit message
3. Add test case (if possible) to prevent regression
4. Document fix if it's non-obvious

---

## Code Review Guidelines

### For Contributors

- Be receptive to feedback
- Ask questions if unclear
- Explain your reasoning
- Don't take criticism personally

### For Reviewers

- Be constructive and specific
- Explain the "why" behind suggestions
- Acknowledge good work
- Focus on code, not person

---

## Community Guidelines

- Be respectful and inclusive
- Help others learn
- Share knowledge
- Give credit where due
- No harassment or discrimination

---

## Questions?

- Open an issue for questions
- Tag issues as `question`
- Check existing issues first
- Ask in discussions (when enabled)

---

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.

---

**Thank you for contributing to Open Wanzer!**
