#ifndef OPENWANZER_FONT_MANAGER_HPP
#define OPENWANZER_FONT_MANAGER_HPP

#include "Raylib.hpp"

#include <map>
#include <string>

namespace fontmanager {

// Font cache that stores pre-rasterized fonts at multiple sizes
class FontCache {
public:
	FontCache();
	~FontCache();

	// Load font at multiple predefined sizes
	void LoadFontAtSizes(const std::string& fontPath);

	// Get font at exact size (returns closest if not available)
	Font GetFont(int size) const;

	// Get default font size (matches raygui TEXT_SIZE)
	int GetDefaultSize() const;

	// Get text spacing
	float GetSpacing() const;

	// Clear all loaded fonts
	void Clear();

	// Check if fonts are loaded
	bool IsLoaded() const;

private:
	std::map<int, Font> fonts_;
	int defaultSize_;
	float spacing_;
	bool isLoaded_;

	// Standard font sizes we support
	static const int kFontSizes[];
	static const int kFontSizeCount;
};

// Global font cache instance
extern FontCache FONT_CACHE;

} // namespace fontmanager

#endif // OPENWANZER_FONT_MANAGER_HPP
