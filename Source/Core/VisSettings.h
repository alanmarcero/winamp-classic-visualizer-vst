#pragma once

namespace wcv {

// The options on Winamp's Classic Skins preferences page. The enum order
// matches the original config values.
enum class VisMode { Off, Spectrum, Oscilloscope };            // config_sa
enum class AnalyzerColoring { Normal, Fire, Line };            // config_safire & 3
enum class ScopeStyle { Dots, Lines, Solid };                  // (config_safire >> 2) & 3
enum class BandWidth { Thick, Thin };                          // config_safire & 32

struct VisSettings {
    VisMode mode = VisMode::Spectrum;
    AnalyzerColoring coloring = AnalyzerColoring::Normal;
    ScopeStyle scopeStyle = ScopeStyle::Lines;
    BandWidth bandWidth = BandWidth::Thick;
    bool peaks = true;
    int falloff = 2;        // config_safalloff, 0 (slowest) to 4 (fastest)
    int peakFalloff = 1;    // config_sa_peak_falloff, 0 to 4
    int refreshDivider = 2; // config_saref: 1 = 70 fps, 2 = 35, 3 = 18, 4 = 9
};

// The defaults in Src/Winamp/config.h.
inline constexpr VisSettings kWinampDefaults {};

} // namespace wcv
