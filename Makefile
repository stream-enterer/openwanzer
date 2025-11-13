# Makefile for Open Wanzer with local raylib

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -I./include
LDFLAGS = -L./lib -Wl,-rpath=./lib
LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11
TARGET = openwanzer
SOURCE = openwanzer.cpp

.PHONY: all clean run check-deps

all: check-deps $(TARGET)

$(TARGET): $(SOURCE)
	@echo "Building Open Wanzer..."
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "Build complete! Run with: ./$(TARGET)"

clean:
	@echo "Cleaning build artifacts..."
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
