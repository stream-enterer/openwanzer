#include "FontManager.hpp"
#include "Raygui.hpp"

#include <algorithm>
#include <cmath>

namespace fontmanager {

// Standard font sizes we'll pre-generate
// Covers all sizes used in the game: 8, 10, 12, 14, 16, 20, 30
const int FontCache::kFontSizes[] = {8, 10, 12, 14, 16, 20, 30};
const int FontCache::kFontSizeCount = sizeof(kFontSizes) / sizeof(kFontSizes[0]);

// Global font cache instance
FontCache FONT_CACHE;

FontCache::FontCache()
    : defaultSize_(10), spacing_(0.0f), isLoaded_(false) {
}

FontCache::~FontCache() {
	Clear();
}

void FontCache::LoadFontAtSizes(const std::string& fontPath) {
	Clear();

	// Load font at each predefined size
	for (int i = 0; i < kFontSizeCount; i++) {
		int size = kFontSizes[i];
		Font font = LoadFontEx(fontPath.c_str(), size, nullptr, 0);

		if (font.texture.id > 0) {
			fonts_[size] = font;
			TraceLog(LOG_INFO, TextFormat("Loaded font at size %d", size));
		} else {
			TraceLog(LOG_WARNING, TextFormat("Failed to load font at size %d", size));
		}
	}

	// Get default size and spacing from raygui
	defaultSize_ = GuiGetStyle(DEFAULT, TEXT_SIZE);
	spacing_ = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);

	isLoaded_ = !fonts_.empty();

	if (isLoaded_) {
		TraceLog(LOG_INFO, TextFormat("Font cache loaded with %d sizes", (int)fonts_.size()));
	}
}

Font FontCache::GetFont(int size) const {
	// If fonts not loaded, return raygui's font
	if (!isLoaded_) {
		return GuiGetFont();
	}

	// Try exact match first
	auto it = fonts_.find(size);
	if (it != fonts_.end()) {
		return it->second;
	}

	// Find closest size
	int closestSize = kFontSizes[0];
	int minDiff = std::abs(size - closestSize);

	for (int i = 1; i < kFontSizeCount; i++) {
		int diff = std::abs(size - kFontSizes[i]);
		if (diff < minDiff) {
			minDiff = diff;
			closestSize = kFontSizes[i];
		}
	}

	// Return closest match
	it = fonts_.find(closestSize);
	if (it != fonts_.end()) {
		return it->second;
	}

	// Fallback to raygui font
	return GuiGetFont();
}

int FontCache::GetDefaultSize() const {
	return defaultSize_;
}

float FontCache::GetSpacing() const {
	return spacing_;
}

void FontCache::Clear() {
	// Unload all fonts
	for (auto& pair : fonts_) {
		if (pair.second.texture.id > 0) {
			UnloadFont(pair.second);
		}
	}
	fonts_.clear();
	isLoaded_ = false;
}

bool FontCache::IsLoaded() const {
	return isLoaded_;
}

} // namespace fontmanager
