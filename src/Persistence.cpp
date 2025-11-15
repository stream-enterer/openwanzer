#include "Config.hpp"
#include "GameState.hpp"
#include "raylib.h"
#include <fstream>
#include <string>

namespace Config {

// Save config to file
void saveConfig(const VideoSettings& settings) {
  std::ofstream configFile("config.txt");
  if (!configFile.is_open()) {
    TraceLog(LOG_WARNING, "Failed to save config.txt");
    return;
  }

  configFile << "resolutionIndex=" << settings.resolutionIndex << "\n";
  configFile << "fullscreen=" << (settings.fullscreen ? 1 : 0) << "\n";
  configFile << "vsync=" << (settings.vsync ? 1 : 0) << "\n";
  configFile << "fpsIndex=" << settings.fpsIndex << "\n";
  configFile << "hexSize=" << settings.hexSize << "\n";
  configFile << "panSpeed=" << settings.panSpeed << "\n";
  configFile << "msaa=" << (settings.msaa ? 1 : 0) << "\n";
  configFile << "guiScaleIndex=" << settings.guiScaleIndex << "\n";
  configFile << "styleTheme=" << settings.styleTheme << "\n";

  configFile.close();
  TraceLog(LOG_INFO, "Config saved to config.txt");
}

// Load config from file
void loadConfig(VideoSettings& settings) {
  std::ifstream configFile("config.txt");
  if (!configFile.is_open()) {
    TraceLog(LOG_INFO, "No config.txt found, creating default config");
    saveConfig(settings);
    return;
  }

  std::string line;
  while (std::getline(configFile, line)) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') continue;

    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) continue;

    std::string key = line.substr(0, equalPos);
    std::string value = line.substr(equalPos + 1);

    try {
      if (key == "resolutionIndex") {
        int val = std::stoi(value);
        if (val >= 0 && val < RESOLUTION_COUNT) {
          settings.resolutionIndex = val;
        }
      } else if (key == "fullscreen") {
        settings.fullscreen = (std::stoi(value) != 0);
      } else if (key == "vsync") {
        settings.vsync = (std::stoi(value) != 0);
      } else if (key == "fpsIndex") {
        int val = std::stoi(value);
        if (val >= 0 && val <= 6) {
          settings.fpsIndex = val;
        }
      } else if (key == "hexSize") {
        float val = std::stof(value);
        if (val >= 20.0f && val <= 80.0f) {
          settings.hexSize = val;
        }
      } else if (key == "panSpeed") {
        float val = std::stof(value);
        if (val >= 1.0f && val <= 20.0f) {
          settings.panSpeed = val;
        }
      } else if (key == "msaa") {
        settings.msaa = (std::stoi(value) != 0);
      } else if (key == "guiScaleIndex") {
        int val = std::stoi(value);
        if (val >= 0 && val < GUI_SCALE_COUNT) {
          settings.guiScaleIndex = val;
        }
      } else if (key == "styleTheme") {
        settings.styleTheme = value;
      }
    } catch (const std::exception& e) {
      // Ignore malformed values
      TraceLog(LOG_WARNING, TextFormat("Failed to parse config value: %s", key.c_str()));
    }
  }

  configFile.close();
  TraceLog(LOG_INFO, "Config loaded from config.txt");
}

} // namespace Config
