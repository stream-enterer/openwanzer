# Quick Start Guide

## Installation (5 minutes)

### 1. Install raylib

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libraylib-dev
```

**Arch Linux:**
```bash
sudo pacman -S raylib
```

**macOS:**
```bash
brew install raylib
```

### 2. Build the game

**Fish shell:**
```fish
./build.fish
```

**Bash:**
```bash
./build.sh
```

**Make:**
```bash
make
```

### 3. Run!
```bash
./pg2_prototype
```

## First Game (2 minutes)

### Playing as Axis (Red)

1. **Click** a red unit to select it
   - Green hexes show where you can move
   - Red hexes show enemies you can attack

2. **Click** a green hex to move there

3. **Click** a red hex to attack that enemy

4. Press **SPACE** to end your turn

### Playing as Allied (Blue)

5. Now it's the blue player's turn
6. Repeat the same steps with blue units

### Tips

- Artillery can shoot 3 hexes away
- Units that don't move get +1 defense (entrenchment)
- Experienced units (with ★) are stronger
- Press arrow keys to pan the camera

## Controls Reference

| Key | Action |
|-----|--------|
| Left Click | Select / Move / Attack |
| Arrow Keys | Pan camera |
| SPACE | End turn |
| ESC | Quit |

## Quick Example

```
1. Click RED tank at top-left
2. Click green hex near it → Tank moves
3. Click red hex with BLUE unit → Attack!
4. Press SPACE → Allied turn
5. Click BLUE unit
6. Attack RED tank back
7. Press SPACE → Next turn
```

## Victory

Capture all the gold-circled hexes or eliminate all enemy units!

## Need Help?

See **README.md** for detailed documentation.
