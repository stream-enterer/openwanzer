#include "Config.h"
#include "raylib.h"
#include "raygui.h"
#include <algorithm>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>

namespace Config {

// Global variables for style themes
std::vector<std::string> AVAILABLE_STYLES;
std::string STYLE_LABELS_STRING;

// Function to discover available styles
void discoverStyles() {
  AVAILABLE_STYLES.clear();
  const char* stylesPath = "resources/styles";

  DIR* dir = opendir(stylesPath);
  if (dir == nullptr) {
    TraceLog(LOG_WARNING, "Failed to open styles directory");
    // Add default style as fallback
    AVAILABLE_STYLES.push_back("default");
    STYLE_LABELS_STRING = "default";
    return;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    // Skip . and ..
    if (entry->d_name[0] == '.') continue;

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
    if (i > 0) STYLE_LABELS_STRING += ";";
    STYLE_LABELS_STRING += AVAILABLE_STYLES[i];
  }

  TraceLog(LOG_INFO, TextFormat("Found %d styles", (int)AVAILABLE_STYLES.size()));
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
  std::string stylePath = "resources/styles/" + themeName + "/style_" + themeName + ".rgs";

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

} // namespace Config
