// Ported from draw_sa in Src/Winamp/draw_sa.cpp and bivis_thread in
// Src/Winamp/SA.cpp, (c) Nullsoft, Inc. Winamp Collaborative License 1.0:
// private use only.

#include "ClassicVis.h"

#include <algorithm>
#include <cstdlib>
#include <ranges>
#include <utility>

namespace wcv {
namespace {

constexpr std::array<int, 5> kBarFalloff { 3, 6, 12, 16, 32 };                 // fo[]
constexpr std::array<float, 5> kPeakFalloff { 1.05f, 1.1f, 1.2f, 1.4f, 1.6f }; // pfo[]

// Bars and the scope start at row 14, but single size shows rows 16 to 31.
// So the bottom 2 rows of every bar are not visible, as in Winamp.
constexpr int kBaseRow = 14;
constexpr int kFirstVisibleRow = kBufferRows - kVisHeight;

constexpr std::uint8_t kBackgroundColor = 0;
constexpr std::uint8_t kDotColor = 1;
constexpr int kBarBottomColor = 17;
constexpr std::uint8_t kPeakColor = 23;
constexpr int kScopeColor = 18;
constexpr float kPeakStartSpeed = 3.0f;

int bandLevel(const SpectrumFrame& spectrum, int x, BandWidth width)
{
    if (width == BandWidth::Thin)
        return spectrum[x];

    // Thick bands average 4 values. The band after the last is the zeroed
    // scope half of Winamp's buffer.
    const int first = x & ~3;
    const auto valueAt = [&spectrum](int band) { return band < kBands ? int(spectrum[band]) : 0; };
    return (valueAt(first) + valueAt(first + 1) + valueAt(first + 2) + valueAt(first + 3)) / 4;
}

int barBaseColor(AnalyzerColoring coloring, int height)
{
    if (coloring == AnalyzerColoring::Fire)
        return height + 2;
    if (coloring == AnalyzerColoring::Line)
        return kBarBottomColor - height;
    return kBarBottomColor;
}

std::pair<int, int> scopeRows(ScopeStyle style, int level, int previous)
{
    if (style == ScopeStyle::Dots)
        return { level, level };
    if (style == ScopeStyle::Solid)
        return level > 7 ? std::pair { 8, level } : std::pair { level, 7 };
    return level >= previous ? std::pair { previous, level } : std::pair { level, previous - 1 };
}

} // namespace

bool ClassicVis::tick(const VisSettings& settings, const SpectrumFrame& spectrum, const ScopeFrame& scope)
{
    if (settings.mode == VisMode::Off) {
        blank();
        return true;
    }

    const bool draw = ++cycle_ >= settings.refreshDivider;
    if (draw) {
        cycle_ = 0;
        drawBackground();
    }
    if (settings.mode == VisMode::Spectrum) {
        stepSpectrum(settings, spectrum, draw);
        return draw;
    }
    if (draw)
        drawScope(settings.scopeStyle, scope);
    return draw;
}

std::uint8_t ClassicVis::pixelAt(int x, int y) const
{
    return pixels_[(kBufferRows - 1 - y) * kVisWidth + x];
}

// draw_sa(NULL, 1): background, and the bars drop to zero.
void ClassicVis::blank()
{
    drawBackground();
    bars_.fill(0);
}

void ClassicVis::drawBackground()
{
    std::ranges::for_each(std::views::iota(kFirstVisibleRow, kBufferRows), [this](int row) {
        const bool dotted = (row - kFirstVisibleRow) % 2 == 0;
        std::ranges::for_each(std::views::iota(0, kVisWidth), [&](int x) {
            pixels_[row * kVisWidth + x] = dotted && x % 2 == 0 ? kDotColor : kBackgroundColor;
        });
    });
}

void ClassicVis::stepSpectrum(const VisSettings& settings, const SpectrumFrame& spectrum, bool draw)
{
    const int barFall = kBarFalloff[std::clamp(settings.falloff, 0, 4)];
    const float peakRate = kPeakFalloff[std::clamp(settings.peakFalloff, 0, 4)];

    std::ranges::for_each(std::views::iota(0, kBands), [&](int x) {
        const int level = std::min(bandLevel(spectrum, x, settings.bandWidth), kMaxLevel);
        const int target = (level << 4) < bars_[x] ? bars_[x] - barFall : level << 4;
        bars_[x] = std::max(target, 0);
        const int height = bars_[x] >> 4;

        if (peaks_[x] <= height * 256) {
            peaks_[x] = height * 256;
            peakSpeeds_[x] = kPeakStartSpeed;
        }
        if (draw && (settings.bandWidth == BandWidth::Thin || (x & 3) != 3))
            drawBar(x, height, settings);

        peaks_[x] = std::max(peaks_[x] - static_cast<int>(peakSpeeds_[x]), 0);
        peakSpeeds_[x] *= peakRate;
    });
}

void ClassicVis::drawBar(int x, int height, const VisSettings& settings)
{
    const int base = barBaseColor(settings.coloring, height);
    const bool gradient = settings.coloring != AnalyzerColoring::Line;
    std::ranges::for_each(std::views::iota(0, height), [&](int y) {
        pixels_[(kBaseRow + y) * kVisWidth + x] = static_cast<std::uint8_t>(gradient ? base - y : base);
    });

    const int peakRow = peaks_[x] / 256;
    if (settings.peaks && peakRow >= 0 && peakRow <= kMaxLevel)
        pixels_[(kBaseRow + peakRow) * kVisWidth + x] = kPeakColor;
}

void ClassicVis::drawScope(ScopeStyle style, const ScopeFrame& scope)
{
    int previous = -1;
    std::ranges::for_each(std::views::iota(0, kBands), [&](int x) {
        const int level = std::clamp(scope[x] + 8, 0, kMaxLevel);
        const auto color = static_cast<std::uint8_t>(std::abs(level / 2 - 4) + kScopeColor);
        const int from = previous < 0 ? level : previous;
        previous = level;
        const auto [low, high] = scopeRows(style, level, from);
        fillColumn(x, kBaseRow + low, kBaseRow + high, color);
    });
}

void ClassicVis::fillColumn(int x, int lowRow, int highRow, std::uint8_t color)
{
    std::ranges::for_each(std::views::iota(lowRow, highRow + 1), [&](int row) {
        pixels_[row * kVisWidth + x] = color;
    });
}

} // namespace wcv
