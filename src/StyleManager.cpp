#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include "Config.hpp"
#include "rl/raygui.h"
#include "rl/raylib.h"

namespace config {

// Global variables for style themes
std::vector<std::string> AVAILABLE_STYLES;
std::string STYLE_LABELS_STRING;
std::string STYLES_PATH; // Path where styles were found

// Function to discover available styles
void discoverStyles() {
	AVAILABLE_STYLES.clear();

	// Try multiple possible paths for the styles directory
	// This handles running from both project root and build directory
	const char* possiblePaths[] = {
	    "resources/styles",   // Running from project root
	    "../resources/styles" // Running from build directory
	};

	const char* stylesPath = nullptr;
	DIR* dir = nullptr;

	// Try each path until we find one that works
	for (const char* path : possiblePaths) {
		dir = opendir(path);
		if (dir != nullptr) {
			stylesPath = path;
			STYLES_PATH = path; // Store the successful path for later use
			break;
		}
	}

	if (dir == nullptr) {
		TraceLog(LOG_WARNING, "Failed to open styles directory (tried multiple paths)");
		// Add default style as fallback
		AVAILABLE_STYLES.push_back("default");
		STYLE_LABELS_STRING = "default";
		STYLES_PATH = "resources/styles"; // Fallback path
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		// Skip . and ..
		if (entry->d_name[0] == '.')
			continue;

		// Check if it's a directory
		std::string fullPath = std::string(stylesPath) + "/" + entry->d_name;
		struct stat statbuf;
		if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
			// Check if .rgs file exists
			std::string rgsPath = fullPath + "/style_" + entry->d_name + ".rgs";
			std::ifstream rgsFile(rgsPath);
			if (rgsFile.good()) {
				AVAILABLE_STYLES.push_back(entry->d_name);
			}
		}
	}
	closedir(dir);

	// Sort styles alphabetically
	std::sort(AVAILABLE_STYLES.begin(), AVAILABLE_STYLES.end());

	// Create labels string
	STYLE_LABELS_STRING.clear();
	for (size_t i = 0; i < AVAILABLE_STYLES.size(); i++) {
		if (i > 0)
			STYLE_LABELS_STRING += ";";
		STYLE_LABELS_STRING += AVAILABLE_STYLES[i];
	}

	TraceLog(LOG_INFO, TextFormat("Found %d styles in %s", (int)AVAILABLE_STYLES.size(), stylesPath));
}

// Get index of a style by name
int getStyleIndex(const std::string& styleName) {
	for (size_t i = 0; i < AVAILABLE_STYLES.size(); i++) {
		if (AVAILABLE_STYLES[i] == styleName) {
			return (int)i;
		}
	}
	return 0; // Default to first style if not found
}

// Load style theme
void loadStyleTheme(const std::string& themeName) {
	// Use the path that was discovered during discoverStyles()
	// This ensures we use the correct path regardless of working directory
	std::string stylePath = STYLES_PATH + "/" + themeName + "/style_" + themeName + ".rgs";

	// Check if file exists
	std::ifstream styleFile(stylePath);
	if (!styleFile.good()) {
		TraceLog(LOG_WARNING, TextFormat("Style file not found: %s", stylePath.c_str()));
		return;
	}
	styleFile.close();

	// Load the style
	GuiLoadStyle(stylePath.c_str());
	TraceLog(LOG_INFO, TextFormat("Style loaded: %s", themeName.c_str()));
}

// Apply GUI scale (currently disabled)
void applyGuiScale(float scale) {
	// GUI scaling functionality has been removed as requested
	// The menu option remains for potential future use
	TraceLog(LOG_INFO, TextFormat("GUI scale setting: %.2f (scaling disabled)", scale));
}

} // namespace config
