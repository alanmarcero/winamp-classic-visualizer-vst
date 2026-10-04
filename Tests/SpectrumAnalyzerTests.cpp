#include "SpectrumAnalyzer.h"
#include "TestSignals.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <functional>

using Catch::Approx;

namespace {

constexpr double kSampleRate = 44100.0;

wcv::SpectrumFrame lastFrameOf(const std::vector<float>& samples)
{
    wcv::SpectrumAnalyzer analyzer;
    wcv::SpectrumFrame last {};
    analyzer.push(samples, samples, [&last](const wcv::SpectrumFrame& frame) { last = frame; });
    return last;
}

int loudestBand(const wcv::SpectrumFrame& frame)
{
    return static_cast<int>(std::ranges::max_element(frame) - frame.begin());
}

} // namespace

TEST_CASE("band edges follow Winamp's semitone warp")
{
    const auto& edges = wcv::SpectrumAnalyzer::bandEdges();

    CHECK(edges[0] == 1.0f);
    CHECK(edges[1] == Approx(1.19272f).epsilon(1e-4));
    CHECK(edges[wcv::kBands] == Approx(252.6413f).epsilon(1e-5));
    CHECK(std::ranges::adjacent_find(edges, std::greater_equal<float> {}) == edges.end());
}

TEST_CASE("a frame needs 512 samples, then comes every 256")
{
    wcv::SpectrumAnalyzer analyzer;
    int frames = 0;
    const auto count = [&frames](const wcv::SpectrumFrame&) { ++frames; };
    const std::vector<float> silence(1024, 0.0f);
    const std::span<const float> samples(silence);

    analyzer.push(samples.first(511), samples.first(511), count);
    CHECK(frames == 0);

    analyzer.push(samples.first(1), samples.first(1), count);
    CHECK(frames == 1);

    analyzer.push(samples.first(512), samples.first(512), count);
    CHECK(frames == 3);
}

TEST_CASE("silence gives empty bands")
{
    const auto frame = lastFrameOf(std::vector<float>(2048, 0.0f));

    CHECK(std::ranges::all_of(frame, [](std::uint8_t band) { return band == 0; }));
}

TEST_CASE("a tone lands in the band for its pitch")
{
    // 1 kHz is FFT bin 11.6 at 44.1 kHz, which is band 24.
    // 8 kHz is bin 92.9, which is band 57.
    const auto low = lastFrameOf(test::sine(1000.0, kSampleRate, 2048, 0.25f));
    const auto high = lastFrameOf(test::sine(8000.0, kSampleRate, 2048, 0.25f));

    CHECK(loudestBand(low) >= 22);
    CHECK(loudestBand(low) <= 27);
    CHECK(loudestBand(high) >= 55);
    CHECK(loudestBand(high) <= 60);
}

TEST_CASE("a full-scale tone pushes its band past the 15-pixel ceiling")
{
    const auto frame = lastFrameOf(test::sine(1000.0, kSampleRate, 2048));

    CHECK(frame[static_cast<std::size_t>(loudestBand(frame))] > wcv::kMaxLevel);
}
