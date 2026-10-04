#include "Parameters.h"

namespace params {
namespace {

// Labels from Winamp's vis right-click menu. The order is the config value.
const juce::StringArray kSpeedLabels { "Slowest", "Slow", "Moderate", "Fast", "Fastest" };

std::unique_ptr<juce::AudioParameterChoice> choice(const char* id, const char* name,
                                                   const juce::StringArray& labels, int defaultIndex)
{
    return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { id, 1 }, name, labels, defaultIndex);
}

int indexOf(const juce::AudioProcessorValueTreeState& state, const char* id)
{
    return static_cast<int>(state.getRawParameterValue(id)->load());
}

} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    constexpr auto defaults = wcv::kWinampDefaults;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(choice(kMode, "Mode", { "Disabled", "Spectrum analyzer", "Oscilloscope" },
                      static_cast<int>(defaults.mode)));
    layout.add(choice(kColoring, "Analyzer coloring", { "Normal", "Fire", "Line" },
                      static_cast<int>(defaults.coloring)));
    layout.add(choice(kScopeStyle, "Oscilloscope style", { "Dots", "Lines", "Solid" },
                      static_cast<int>(defaults.scopeStyle)));
    layout.add(choice(kBandWidth, "Band width", { "Thick bands", "Thin bands" },
                      static_cast<int>(defaults.bandWidth)));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { kPeaks, 1 }, "Show peaks", defaults.peaks));
    layout.add(choice(kFalloff, "Analyzer falloff", kSpeedLabels, defaults.falloff));
    layout.add(choice(kPeakFalloff, "Peaks falloff", kSpeedLabels, defaults.peakFalloff));
    layout.add(choice(kRefresh, "Refresh rate", { "Full (70 fps)", "Half (35 fps)", "Quarter (18 fps)", "Eighth (9 fps)" },
                      defaults.refreshDivider - 1));
    return layout;
}

wcv::VisSettings readSettings(const juce::AudioProcessorValueTreeState& state)
{
    return {
        .mode = static_cast<wcv::VisMode>(indexOf(state, kMode)),
        .coloring = static_cast<wcv::AnalyzerColoring>(indexOf(state, kColoring)),
        .scopeStyle = static_cast<wcv::ScopeStyle>(indexOf(state, kScopeStyle)),
        .bandWidth = static_cast<wcv::BandWidth>(indexOf(state, kBandWidth)),
        .peaks = indexOf(state, kPeaks) != 0,
        .falloff = indexOf(state, kFalloff),
        .peakFalloff = indexOf(state, kPeakFalloff),
        .refreshDivider = indexOf(state, kRefresh) + 1,
    };
}

} // namespace params
