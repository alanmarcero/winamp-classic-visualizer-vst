#pragma once

namespace wcv {

inline constexpr int kBands = 75;
inline constexpr int kFftSize = 512;
inline constexpr int kFftBins = kFftSize / 2;
inline constexpr int kHopSize = 256;
inline constexpr int kScopeBlock = 576;
inline constexpr int kMaxLevel = 15;

// The classic vis area is 76 x 16 px. Winamp draws it into a bottom-up
// 8-bit DIB of 32 rows. Single size shows rows 16 to 31 only.
inline constexpr int kVisWidth = 76;
inline constexpr int kVisHeight = 16;
inline constexpr int kBufferRows = 32;

// Refresh labels in Winamp's options: full rate is 70 fps.
inline constexpr int kTickHz = 70;

} // namespace wcv
