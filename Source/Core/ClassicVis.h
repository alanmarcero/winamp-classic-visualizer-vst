#pragma once

#include "Oscilloscope.h"
#include "SpectrumAnalyzer.h"
#include "VisConstants.h"
#include "VisSettings.h"

#include <array>
#include <cstdint>

namespace wcv {

// Matches the single-size, non-windowshade paths of draw_sa
// (Src/Winamp/draw_sa.cpp) and the refresh logic of bivis_thread
// (Src/Winamp/SA.cpp). Pixels are palette indices.
class ClassicVis {
public:
    // One pass of the vis thread. Call it kTickHz times a second.
    // Returns true when the pixels changed.
    bool tick(const VisSettings& settings, const SpectrumFrame& spectrum, const ScopeFrame& scope);

    // Palette index at (x, y) of the 76 x 16 vis area. y = 0 is the top.
    std::uint8_t pixelAt(int x, int y) const;

private:
    void blank();
    void drawBackground();
    void stepSpectrum(const VisSettings& settings, const SpectrumFrame& spectrum, bool draw);
    void drawBar(int x, int height, const VisSettings& settings);
    void drawScope(ScopeStyle style, const ScopeFrame& scope);
    void fillColumn(int x, int lowRow, int highRow, std::uint8_t color);

    std::array<int, kBands> bars_ {};          // bx: level << 4
    std::array<int, kBands> peaks_ {};         // t_bx: level * 256
    std::array<float, kBands> peakSpeeds_ {};  // t_vx
    std::array<std::uint8_t, kVisWidth * kBufferRows> pixels_ {};
    int cycle_ = 0;
};

} // namespace wcv
