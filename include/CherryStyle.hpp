#ifndef OPENWANZER_CHERRY_STYLE_HPP
#define OPENWANZER_CHERRY_STYLE_HPP

#include "Raylib.hpp"

// Cherry UI Style - Fixed style constants
// Based on style_cherry.txt.rgs from resources/styles/cherry/

namespace cherrystyle {

// Font configuration
constexpr int kFontSize = 15;
constexpr int kFontSpacing = 0;
constexpr int kTextLineSpacing = 22;

// Global font (loaded at initialization)
extern Font CHERRY_FONT;

// Path to style resources
extern const char* kStylePath;
extern const char* kFontPath;

// Color palette (from style_cherry.txt.rgs)
constexpr Color kBorderColorNormal = Color {0xda, 0x57, 0x57, 0xff};
constexpr Color kBaseColorNormal = Color {0x75, 0x32, 0x33, 0xff};
constexpr Color kTextColorNormal = Color {0xe1, 0x73, 0x73, 0xff};

constexpr Color kBorderColorFocused = Color {0xfa, 0xaa, 0x97, 0xff};
constexpr Color kBaseColorFocused = Color {0xe0, 0x62, 0x62, 0xff};
constexpr Color kTextColorFocused = Color {0xfd, 0xb4, 0xaa, 0xff};

constexpr Color kBorderColorPressed = Color {0xe0, 0x3c, 0x46, 0xff};
constexpr Color kBaseColorPressed = Color {0x5b, 0x1e, 0x20, 0xff};
constexpr Color kTextColorPressed = Color {0xc2, 0x47, 0x4f, 0xff};

constexpr Color kBorderColorDisabled = Color {0xa1, 0x92, 0x92, 0xff};
constexpr Color kBaseColorDisabled = Color {0x70, 0x60, 0x60, 0xff};
constexpr Color kTextColorDisabled = Color {0x9e, 0x85, 0x85, 0xff};

constexpr Color kLineColor = Color {0xfb, 0x81, 0x70, 0xff};
constexpr Color kBackgroundColor = Color {0x3a, 0x17, 0x20, 0xff};

// Initialization function
void InitializeCherryStyle();

// Cleanup function
void UnloadCherryStyle();

} // namespace cherrystyle

#endif // OPENWANZER_CHERRY_STYLE_HPP
