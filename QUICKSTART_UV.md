# Quick Start with UV

## What is uv?
`uv` is an extremely fast Python package installer and resolver, written in Rust. It's 10-100x faster than pip!

## Installation

### Linux/Mac (Fish shell compatible):
```fish
curl -LsSf https://astral.sh/uv/install.sh | sh
```

### Windows:
```powershell
powershell -c "irm https://astral.sh/uv/install.ps1 | iex"
```

## Running the Game

Once you have uv installed, it's super simple:

```bash
# Navigate to the panzer_proto directory
cd panzer_proto

# Run the game (uv will automatically install dependencies)
uv run main.py
```

That's it! No need to manually install arcade or create virtual environments. 
uv handles everything automatically.

## How it Works

When you run `uv run main.py`, uv:
1. Reads the `pyproject.toml` file
2. Creates an isolated environment
3. Installs the required dependencies (arcade)
4. Runs your game

## Other Useful uv Commands

```bash
# Just install dependencies without running
uv pip install -r pyproject.toml

# Create a virtual environment
uv venv

# Sync dependencies
uv sync

# Add a new dependency to pyproject.toml
uv add <package-name>
```

## Why Use uv Instead of pip?

- ⚡ **Much faster** - 10-100x faster than pip
- 🔒 **Consistent** - Deterministic dependency resolution
- 🎯 **Simple** - Handles virtual environments automatically
- 💾 **Efficient** - Shared package cache across projects
- 🦀 **Modern** - Written in Rust for performance

## More Info

- uv documentation: https://docs.astral.sh/uv/
- uv GitHub: https://github.com/astral-sh/uv
