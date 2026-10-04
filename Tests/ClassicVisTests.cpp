// Expected columns come from draw_sa in Src/Winamp/draw_sa.cpp. A column
// is listed top (y = 0) to bottom (y = 15). Palette indices: 0 background,
// 1 grid dot, 2 to 17 bar colours, 18 to 22 scope, 23 peak.

#include "ClassicVis.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <ranges>

namespace {

using Column = std::array<std::uint8_t, wcv::kVisHeight>;

constexpr wcv::VisSettings kEveryTick { .refreshDivider = 1 };
constexpr wcv::ScopeFrame kFlatScope {};
constexpr wcv::SpectrumFrame kSilence {};

wcv::SpectrumFrame spectrumOf(std::uint8_t level)
{
    wcv::SpectrumFrame frame {};
    frame.fill(level);
    return frame;
}

wcv::ScopeFrame scopeOf(std::int8_t level)
{
    wcv::ScopeFrame frame {};
    frame.fill(level);
    return frame;
}

Column columnAt(const wcv::ClassicVis& vis, int x)
{
    Column column {};
    std::ranges::transform(std::views::iota(0, wcv::kVisHeight), column.begin(),
                           [&](int y) { return vis.pixelAt(x, y); });
    return column;
}

Column spectrumColumn(const wcv::VisSettings& settings, const wcv::SpectrumFrame& frame, int x)
{
    wcv::ClassicVis vis;
    vis.tick(settings, frame, kFlatScope);
    return columnAt(vis, x);
}

Column scopeColumn(wcv::ScopeStyle style, const wcv::ScopeFrame& frame, int x)
{
    wcv::ClassicVis vis;
    vis.tick({ .mode = wcv::VisMode::Oscilloscope, .scopeStyle = style, .refreshDivider = 1 }, kSilence, frame);
    return columnAt(vis, x);
}

// A full bar, then silence for the given number of ticks.
wcv::ClassicVis afterRelease(const wcv::VisSettings& settings, int silentTicks)
{
    wcv::ClassicVis vis;
    vis.tick(settings, spectrumOf(255), kFlatScope);
    std::ranges::for_each(std::views::iota(0, silentTicks), [&](int) { vis.tick(settings, kSilence, kFlatScope); });
    return vis;
}

long peakY(const wcv::ClassicVis& vis)
{
    const auto column = columnAt(vis, 0);
    return std::ranges::find(column, std::uint8_t { 23 }) - column.begin();
}

} // namespace

TEST_CASE("a full bar shows 13 pixels with its peak above, because rows 14 and 15 are hidden")
{
    CHECK(spectrumColumn(kEveryTick, spectrumOf(255), 0)
          == Column { 0, 1, 23, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
}

TEST_CASE("the three analyzer colorings")
{
    const auto halfBar = spectrumOf(8);

    CHECK(spectrumColumn(kEveryTick, halfBar, 0)
          == Column { 0, 1, 0, 1, 0, 1, 0, 1, 0, 23, 10, 11, 12, 13, 14, 15 });
    CHECK(spectrumColumn({ .coloring = wcv::AnalyzerColoring::Fire, .refreshDivider = 1 }, halfBar, 0)
          == Column { 0, 1, 0, 1, 0, 1, 0, 1, 0, 23, 3, 4, 5, 6, 7, 8 });
    CHECK(spectrumColumn({ .coloring = wcv::AnalyzerColoring::Line, .refreshDivider = 1 }, halfBar, 0)
          == Column { 0, 1, 0, 1, 0, 1, 0, 1, 0, 23, 9, 9, 9, 9, 9, 9 });
}

TEST_CASE("peaks can be turned off")
{
    CHECK(spectrumColumn({ .peaks = false, .refreshDivider = 1 }, spectrumOf(8), 0)
          == Column { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 10, 11, 12, 13, 14, 15 });
}

TEST_CASE("thick bands leave every fourth column empty and thin bands fill it")
{
    CHECK(spectrumColumn(kEveryTick, spectrumOf(255), 3) == Column {});
    CHECK(spectrumColumn({ .bandWidth = wcv::BandWidth::Thin, .refreshDivider = 1 }, spectrumOf(255), 3)
          == Column { 0, 0, 23, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
}

TEST_CASE("thick bands average four values and thin bands use their own")
{
    wcv::SpectrumFrame oneLoud {};
    oneLoud[0] = 32;

    // (32 + 0 + 0 + 0) / 4 = 8, against 32 clamped to 15.
    CHECK(spectrumColumn(kEveryTick, oneLoud, 0) == spectrumColumn(kEveryTick, spectrumOf(8), 0));
    CHECK(spectrumColumn({ .bandWidth = wcv::BandWidth::Thin, .refreshDivider = 1 }, oneLoud, 0)
          == spectrumColumn(kEveryTick, spectrumOf(255), 0));
}

TEST_CASE("bars fall 12/16 px a tick on Moderate and 32/16 px on Fastest")
{
    // Moderate: 240 - 12 = 228, height 14. Fastest: 240 - 32 = 208, height 13.
    // The peak has fallen 3/256 px, so it shows one row lower.
    CHECK(columnAt(afterRelease(kEveryTick, 1), 0)
          == Column { 0, 1, 0, 23, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
    CHECK(columnAt(afterRelease({ .falloff = 4, .refreshDivider = 1 }, 1), 0)
          == Column { 0, 1, 0, 23, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
}

TEST_CASE("a faster peak falloff drops the peak sooner")
{
    const auto slowest = afterRelease({ .peakFalloff = 0, .refreshDivider = 1 }, 12);
    const auto fastest = afterRelease({ .peakFalloff = 4, .refreshDivider = 1 }, 12);

    CHECK(peakY(slowest) == 3);
    CHECK(peakY(fastest) > peakY(slowest));
}

TEST_CASE("the refresh rate skips draws but not the falloff")
{
    wcv::ClassicVis vis;
    const wcv::VisSettings halfRate { .refreshDivider = 2 };

    CHECK_FALSE(vis.tick(halfRate, spectrumOf(255), kFlatScope));
    CHECK(vis.tick(halfRate, kSilence, kFlatScope));
    CHECK(columnAt(vis, 0) == Column { 0, 1, 0, 23, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
}

TEST_CASE("disabled shows the empty grid")
{
    wcv::ClassicVis vis;
    vis.tick(kEveryTick, spectrumOf(255), kFlatScope);

    CHECK(vis.tick({ .mode = wcv::VisMode::Off }, spectrumOf(255), kFlatScope));
    CHECK(columnAt(vis, 0) == Column { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 });
}

TEST_CASE("the scope draws a flat signal on the middle row")
{
    CHECK(scopeColumn(wcv::ScopeStyle::Dots, kFlatScope, 0)
          == Column { 0, 1, 0, 1, 0, 1, 0, 1, 0, 18, 0, 1, 0, 1, 0, 1 });
}

TEST_CASE("the scope styles: dots, lines from the last point, solid to the middle")
{
    wcv::ScopeFrame step {};
    step[1] = 4;

    CHECK(scopeColumn(wcv::ScopeStyle::Dots, step, 1) == Column { 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
    CHECK(scopeColumn(wcv::ScopeStyle::Lines, step, 1) == Column { 0, 0, 0, 0, 0, 20, 20, 20, 20, 20, 0, 0, 0, 0, 0, 0 });
    CHECK(scopeColumn(wcv::ScopeStyle::Solid, scopeOf(-8), 1) == Column { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 22, 22, 22, 22, 22 });
}
