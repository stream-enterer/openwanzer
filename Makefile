# Makefile for Open Wanzer with local raylib and Dear ImGui

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -I./include -I./include/imgui
LDFLAGS = -L./lib -Wl,-rpath=./lib
LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11
TARGET = openwanzer
SOURCE = openwanzer.cpp
IMGUI_SOURCES = include/imgui/imgui.cpp include/imgui/imgui_draw.cpp include/imgui/imgui_tables.cpp include/imgui/imgui_widgets.cpp include/rlImGui_simple.cpp
IMGUI_OBJECTS = $(IMGUI_SOURCES:.cpp=.o)

.PHONY: all clean run check-deps

all: check-deps $(TARGET)

$(TARGET): $(SOURCE) $(IMGUI_OBJECTS)
	@echo "Building Open Wanzer..."
	$(CXX) $(CXXFLAGS) $(SOURCE) $(IMGUI_OBJECTS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "Build complete! Run with: ./$(TARGET)"

include/imgui/%.o: include/imgui/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

include/%.o: include/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET) $(IMGUI_OBJECTS)
	@echo "Clean complete!"

run: $(TARGET)
	./$(TARGET)

check-deps:
	@echo "Checking dependencies..."
	@test -f lib/libraylib.a -o -f lib/libraylib.so || (echo "✗ raylib not found in ./lib/" && exit 1)
	@test -f include/raylib.h || (echo "✗ raylib.h not found in ./include/" && exit 1)
	@test -f include/imgui/imgui.h || (echo "✗ imgui.h not found in ./include/imgui/" && exit 1)
	@test -f include/rlImGui.h || (echo "✗ rlImGui.h not found in ./include/" && exit 1)
	@echo "✓ All dependencies found"
