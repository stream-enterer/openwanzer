# Makefile for Open Wanzer (Multi-File Architecture)

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -I./include -I./src
LDFLAGS = -L./lib -Wl,-rpath=./lib
LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11
TARGET = openwanzer

# Find all .cpp files in src/ recursively
SOURCES = $(shell find src -name '*.cpp')
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean run check-deps

all: check-deps $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking Open Wanzer..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Pattern rule for compiling .cpp files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	find src -name '*.o' -delete
	rm -f $(TARGET)
	@echo "Clean complete!"

run: $(TARGET)
	./$(TARGET)

check-deps:
	@echo "Checking dependencies..."
	@test -f lib/libraylib.a -o -f lib/libraylib.so || (echo "✗ raylib not found in ./lib/" && exit 1)
	@test -f include/raylib.h || (echo "✗ raylib.h not found in ./include/" && exit 1)
	@test -f include/raygui.h || (echo "✗ raygui.h not found in ./include/" && exit 1)
	@echo "✓ All dependencies found"
