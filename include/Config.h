#ifndef OPENWANZER_CONFIG_H
#define OPENWANZER_CONFIG_H

#include <string>
#include <vector>
#include "GameState.h"

namespace Config {

// Global variables for style themes
extern std::vector<std::string> AVAILABLE_STYLES;
extern std::string STYLE_LABELS_STRING;

// Style discovery and loading
void discoverStyles();
int getStyleIndex(const std::string& styleName);
void loadStyleTheme(const std::string& themeName);

// Configuration persistence
void saveConfig(const VideoSettings& settings);
void loadConfig(VideoSettings& settings);

// GUI scaling
void applyGuiScale(float scale);

} // namespace Config

#endif // OPENWANZER_CONFIG_H
