#pragma once

#include "VisSettings.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace params {

inline constexpr auto kMode = "mode";
inline constexpr auto kColoring = "coloring";
inline constexpr auto kScopeStyle = "scopeStyle";
inline constexpr auto kBandWidth = "bandWidth";
inline constexpr auto kPeaks = "peaks";
inline constexpr auto kFalloff = "falloff";
inline constexpr auto kPeakFalloff = "peakFalloff";
inline constexpr auto kRefresh = "refresh";

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

wcv::VisSettings readSettings(const juce::AudioProcessorValueTreeState& state);

} // namespace params
