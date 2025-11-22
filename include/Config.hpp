#ifndef OPENWANZER_CONFIG_HPP
#define OPENWANZER_CONFIG_HPP

#include "GameState.hpp"

namespace config {

// Configuration persistence
void saveConfig(const VideoSettings& settings);
void loadConfig(VideoSettings& settings);

} // namespace config

#endif // OPENWANZER_CONFIG_HPP
