# ==============================================================================
# Open Wanzer - Makefile
# ==============================================================================

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I./include -I./src
CXXFLAGS_DEBUG = -std=c++17 -g -Wall -Wextra -I./include -I./src
LDFLAGS = -L./lib -Wl,-rpath=./lib
LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = openwanzer

# Source files
SOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS_DEBUG = $(SOURCES:.cpp=.debug.o)

# ==============================================================================
# Phony Targets
# ==============================================================================

.PHONY: all clean run check-deps debug format help install uninstall

# ==============================================================================
# Default Target
# ==============================================================================

all: check-deps $(TARGET)

# ==============================================================================
# Build Targets
# ==============================================================================

$(TARGET): $(OBJECTS)
	@echo "Linking Open Wanzer (Release)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "✓ Build complete! Run with: ./$(TARGET)"

debug: CXXFLAGS = $(CXXFLAGS_DEBUG)
debug: $(OBJECTS_DEBUG)
	@echo "Linking Open Wanzer (Debug)..."
	$(CXX) $(OBJECTS_DEBUG) -o $(TARGET)-debug $(LDFLAGS) $(LIBS)
	@echo "✓ Debug build complete! Run with: ./$(TARGET)-debug"

# Pattern rules for compiling .cpp files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.debug.o: %.cpp
	@echo "Compiling $< (debug)..."
	$(CXX) $(CXXFLAGS_DEBUG) -c $< -o $@

# ==============================================================================
# Utility Targets
# ==============================================================================

clean:
	@echo "Cleaning build artifacts..."
	@find $(SRC_DIR) -name '*.o' -delete
	@find $(SRC_DIR) -name '*.debug.o' -delete
	@rm -f $(TARGET) $(TARGET)-debug
	@echo "✓ Clean complete!"

run: $(TARGET)
	@echo "Running Open Wanzer..."
	@./$(TARGET)

check-deps:
	@echo "Checking dependencies..."
	@test -f lib/libraylib.a -o -f lib/libraylib.so || (echo "✗ raylib not found in ./lib/" && exit 1)
	@test -f include/raylib.h || (echo "✗ raylib.h not found in ./include/" && exit 1)
	@test -f include/raygui.h || (echo "✗ raygui.h not found in ./include/" && exit 1)
	@echo "✓ All dependencies found"

format:
	@echo "Formatting code with clang-format..."
	@find $(SRC_DIR) -name '*.cpp' -o -name '*.h' | xargs clang-format -i
	@echo "✓ Formatting complete!"

help:
	@echo "Open Wanzer - Makefile Targets"
	@echo "================================"
	@echo "  all          - Build release version (default)"
	@echo "  debug        - Build debug version with symbols"
	@echo "  clean        - Remove all build artifacts"
	@echo "  run          - Build and run the game"
	@echo "  check-deps   - Verify all dependencies are present"
	@echo "  format       - Format code with clang-format"
	@echo "  install      - Install to /usr/local (requires sudo)"
	@echo "  uninstall    - Remove from /usr/local (requires sudo)"
	@echo "  help         - Show this help message"

install: $(TARGET)
	@echo "Installing Open Wanzer to /usr/local/bin..."
	@install -D -m 755 $(TARGET) /usr/local/bin/$(TARGET)
	@echo "✓ Installation complete!"

uninstall:
	@echo "Uninstalling Open Wanzer from /usr/local/bin..."
	@rm -f /usr/local/bin/$(TARGET)
	@echo "✓ Uninstallation complete!"
