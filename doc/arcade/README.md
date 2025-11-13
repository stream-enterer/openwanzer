# Python Arcade Documentation Reference

This folder contains the official Python Arcade library documentation for quick reference during development.

## Documentation Structure

### 📚 Core Documentation

#### Getting Started (`get_started/`)
- **install.rst** - Installation instructions for Arcade
- **arcade_book.rst** - Arcade book reference

#### Programming Guide (`programming_guide/`)
Essential guides for common Arcade features:
- **camera.rst** - Camera2D system and viewport management
- **event_loop.rst** - Game loop and update cycle
- **keyboard.rst** - Keyboard input handling
- **sound.rst** - Audio and sound effects
- **gui/** - GUI system (UIManager, layouts, widgets)
- **sprites/** - Sprite system and management
- **textures.rst** - Texture loading and management
- **texture_atlas.rst** - Texture atlas optimization
- **performance_tips.rst** - Performance optimization
- **opengl_notes.rst** - OpenGL rendering details
- **resource_handlers.rst** - Resource file handling

#### API Documentation (`api_docs/`)
- **api/** - Complete API reference
- **gl/** - OpenGL specific APIs

#### Tutorials (`tutorials/`)
Step-by-step tutorials for specific features:
- **platform_tutorial/** - Platformer game tutorial
- **views/** - View/screen management
- **gui/** - GUI system tutorial
- **menu/** - Menu creation
- **lights/** - Lighting effects
- **framebuffer/** - Advanced rendering
- **shader_*** - Shader programming tutorials
- And more specialized topics...

## Quick Reference for Common Tasks

### Camera & Viewport
See: `programming_guide/camera.rst`
- Camera2D positioning and movement
- Viewport setup and constraints
- World space vs screen space

### GUI Layout
See: `programming_guide/gui/`
- UIManager setup
- Layouts: UIBoxLayout, UIAnchorLayout, UIGridLayout
- Widgets: UILabel, UIButton, etc.
- Size hints and responsive design

### Drawing & Rendering
See: `programming_guide/opengl_notes.rst`, `api_docs/`
- Modern drawing functions (LRBT format)
- Shape drawing (polygons, circles, rectangles)
- Text rendering best practices
- Performance considerations

### Input Handling
See: `programming_guide/keyboard.rst`
- Keyboard events
- Mouse events
- Event handlers

### Performance
See: `programming_guide/performance_tips.rst`
- Sprite batching
- Texture atlases
- Efficient text rendering
- Draw call optimization

## How to Use This Documentation

When working on a feature, search this folder for relevant documentation:

**Example searches:**
```bash
# Find all camera-related docs
grep -r "Camera2D" doc/arcade/

# Find viewport documentation
grep -r "viewport" doc/arcade/programming_guide/

# Find GUI layout examples
grep -r "UIBoxLayout" doc/arcade/
```

**For Claude Code users:**
- Use the Grep or Read tools to search documentation
- Reference specific .rst files when implementing features
- Cross-reference programming_guide/ and api_docs/ for complete understanding

## Documentation Format

Most files are in reStructuredText (.rst) format, a readable markup language commonly used with Python projects. Read them directly as they are designed to be human-readable.

---

**Note:** This is a snapshot of the Python Arcade documentation for offline reference. For the latest docs, visit: https://api.arcade.academy/
