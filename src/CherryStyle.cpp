#include "CherryStyle.hpp"
#include "Raygui.hpp"
#include "Raylib.hpp"

#include <cstdio>
#include <cstdlib>

namespace cherrystyle {

// Global font instance
Font CHERRY_FONT = {0};

// Paths to style resources
const char* kStylePath = "resources/styles/cherry/style_cherry.txt.rgs";
const char* kFontPath = "resources/styles/cherry/Westington.ttf";

// Alternative paths for running from build directory
const char* kStylePathAlt = "../resources/styles/cherry/style_cherry.txt.rgs";
const char* kFontPathAlt = "../resources/styles/cherry/Westington.ttf";

void InitializeCherryStyle() {
	// Try loading style file from multiple locations
	const char* stylePath = kStylePath;
	FILE* testFile = fopen(stylePath, "r");
	if (!testFile) {
		stylePath = kStylePathAlt;
		testFile = fopen(stylePath, "r");
	}
	if (testFile) {
		fclose(testFile);
		GuiLoadStyle(stylePath);
		TraceLog(LOG_INFO, TextFormat("Cherry style loaded from: %s", stylePath));
	} else {
		TraceLog(LOG_WARNING, "Failed to load Cherry style file (tried multiple paths)");
	}

	// Load font at size 15
	const char* fontPath = kFontPath;
	testFile = fopen(fontPath, "r");
	if (!testFile) {
		fontPath = kFontPathAlt;
		testFile = fopen(fontPath, "r");
	}
	if (testFile) {
		fclose(testFile);
		CHERRY_FONT = LoadFontEx(fontPath, kFontSize, nullptr, 0);
		if (CHERRY_FONT.texture.id > 0) {
			// Set as raygui font
			GuiSetFont(CHERRY_FONT);
			TraceLog(LOG_INFO, TextFormat("Cherry font loaded from: %s at size %d", fontPath, kFontSize));
		} else {
			TraceLog(LOG_WARNING, "Failed to load Cherry font texture");
		}
	} else {
		TraceLog(LOG_WARNING, "Failed to find Cherry font file (tried multiple paths)");
	}
}

void UnloadCherryStyle() {
	if (CHERRY_FONT.texture.id > 0) {
		UnloadFont(CHERRY_FONT);
		CHERRY_FONT = {0};
	}
}

} // namespace cherrystyle
