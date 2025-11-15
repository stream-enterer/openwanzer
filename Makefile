# ==============================================================================
# Open Wanzer - Makefile (CMake Wrapper)
# ==============================================================================
#
# This Makefile provides convenient shortcuts to CMake commands.
# All builds are performed in the build/ directory.
#
# ==============================================================================

.PHONY: all configure build debug release clean run format install uninstall help check-deps

# Build directory
BUILD_DIR = build

# ==============================================================================
# Default Target
# ==============================================================================

all: release

# ==============================================================================
# Configuration Targets
# ==============================================================================

configure:
	@echo "Configuring Open Wanzer with CMake..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release ..
	@echo "✓ Configuration complete!"

configure-debug:
	@echo "Configuring Open Wanzer (Debug) with CMake..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug ..
	@echo "✓ Debug configuration complete!"

# ==============================================================================
# Build Targets
# ==============================================================================

build: configure
	@echo "Building Open Wanzer..."
	@cmake --build $(BUILD_DIR)
	@echo "✓ Build complete! Binary: $(BUILD_DIR)/openwanzer"

release: configure
	@echo "Building Open Wanzer (Release)..."
	@cmake --build $(BUILD_DIR) --config Release
	@echo "✓ Release build complete! Run with: ./$(BUILD_DIR)/openwanzer"

debug: configure-debug
	@echo "Building Open Wanzer (Debug)..."
	@cmake --build $(BUILD_DIR) --config Debug
	@echo "✓ Debug build complete! Run with: ./$(BUILD_DIR)/openwanzer"

# ==============================================================================
# Utility Targets
# ==============================================================================

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@find src -name '*.o' -delete 2>/dev/null || true
	@find src -name '*.debug.o' -delete 2>/dev/null || true
	@rm -f openwanzer openwanzer-debug 2>/dev/null || true
	@echo "✓ Clean complete!"

run: build
	@echo "Running Open Wanzer..."
	@cd $(BUILD_DIR) && ./openwanzer

check-deps:
	@echo "Checking dependencies..."
	@test -f lib/libraylib.a -o -f lib/libraylib.so || (echo "✗ raylib not found in ./lib/" && exit 1)
	@test -f include/raylib.h || (echo "✗ raylib.h not found in ./include/" && exit 1)
	@test -f include/raygui.h || (echo "✗ raygui.h not found in ./include/" && exit 1)
	@echo "✓ All dependencies found"

format:
	@echo "Formatting code with clang-format..."
	@find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i 2>/dev/null || echo "clang-format not found, skipping..."
	@echo "✓ Formatting complete!"

# ==============================================================================
# Installation Targets
# ==============================================================================

install: release
	@echo "Installing Open Wanzer to /usr/local/bin..."
	@sudo install -D -m 755 $(BUILD_DIR)/openwanzer /usr/local/bin/openwanzer
	@echo "✓ Installation complete!"

uninstall:
	@echo "Uninstalling Open Wanzer from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/openwanzer
	@echo "✓ Uninstallation complete!"

# ==============================================================================
# Help Target
# ==============================================================================

help:
	@echo "Open Wanzer - CMake Build System"
	@echo "=================================="
	@echo ""
	@echo "Build Targets:"
	@echo "  all          - Build release version (default)"
	@echo "  release      - Build release version"
	@echo "  debug        - Build debug version with symbols"
	@echo "  build        - Build with current configuration"
	@echo ""
	@echo "Configuration:"
	@echo "  configure       - Configure CMake (Release)"
	@echo "  configure-debug - Configure CMake (Debug)"
	@echo ""
	@echo "Utilities:"
	@echo "  clean        - Remove all build artifacts"
	@echo "  run          - Build and run the game"
	@echo "  check-deps   - Verify all dependencies are present"
	@echo "  format       - Format code with clang-format"
	@echo ""
	@echo "Installation:"
	@echo "  install      - Install to /usr/local (requires sudo)"
	@echo "  uninstall    - Remove from /usr/local (requires sudo)"
	@echo ""
	@echo "Help:"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Advanced (CMake):"
	@echo "  Use CMake directly for more options:"
	@echo "    mkdir build && cd build"
	@echo "    cmake .."
	@echo "    cmake --build ."
