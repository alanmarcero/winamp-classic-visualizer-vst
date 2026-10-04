#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace wcv {

using Rgb = std::array<std::uint8_t, 3>;
using Palette = std::array<Rgb, 24>;

// The built-in vis colours from Src/Winamp/draw.cpp. 0 is the background,
// 1 the grid dots, 2 to 17 the bars (top to bottom), 18 to 22 the scope,
// and 23 the peaks.
inline constexpr Palette kDefaultPalette { {
    { 0, 0, 0 },       { 24, 24, 41 },    { 239, 49, 16 },   { 206, 41, 16 },
    { 214, 90, 0 },    { 214, 102, 0 },   { 214, 115, 0 },   { 198, 123, 8 },
    { 222, 165, 24 },  { 214, 181, 33 },  { 189, 222, 41 },  { 148, 222, 33 },
    { 41, 206, 16 },   { 50, 190, 16 },   { 57, 181, 16 },   { 49, 156, 8 },
    { 41, 148, 0 },    { 24, 132, 8 },    { 255, 255, 255 }, { 214, 214, 222 },
    { 181, 189, 189 }, { 160, 170, 175 }, { 148, 156, 165 }, { 150, 150, 150 },
} };

// Port of the viscolor.txt reader in Src/Winamp/draw.cpp. Colours the
// file does not set keep their defaults.
Palette parseViscolor(std::string_view text);

} // namespace wcv
