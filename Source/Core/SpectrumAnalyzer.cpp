// Ported from Src/Winamp/SABuffer.cpp and Src/Winamp/classic_vis.cpp,
// (c) Nullsoft, Inc. Winamp Collaborative License 1.0: private use only.

#include "SpectrumAnalyzer.h"

#include "Fft.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <numbers>
#include <numeric>
#include <ranges>

namespace wcv {
namespace {

constexpr float kSpecScale = 0.5f;
constexpr int kLastBin = kFftBins - 1;

// Winamp's 2^y approximation. It sets the band edges, so std::pow is not used.
float fpow2(float y)
{
    const int integer = static_cast<int>(std::lrint(std::floor(y)));
    const float frac = y - static_cast<float>(integer);
    const float whole = std::bit_cast<float>((integer + 127) << 23);
    return whole * (0.33977f * frac * frac + (1.0f - 0.33977f) * frac + 1.0f);
}

float fastmin(float x, float b)
{
    const float difference = b - x;
    return b - (difference + std::fabs(difference)) * 0.5f;
}

// 4-point, 3rd-order Hermite (x-form), as in classic_vis.cpp.
float hermite(float x, float y0, float y1, float y2, float y3)
{
    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
    const float c2 = y0 - y1 + c1 - c3;
    return ((c3 * x + c2) * x + c1) * x + c0;
}

// The do/while in makeSpecData: Hermite for the part-bin at the start,
// whole bins in the middle, and the part-bin at the end.
float bandValue(const std::array<float, kFftSize>& magnitudes, float start, float end)
{
    const int first = static_cast<int>(std::lrint(std::floor(start)));
    const int last = std::min(static_cast<int>(std::lrint(std::floor(end))), kLastBin);
    const float c = first < kLastBin ? magnitudes[first + 1] : 0.0f;
    const float d = first < kLastBin - 1 ? magnitudes[first + 2] : 0.0f;
    const float head = hermite(start - first, magnitudes[first - 1], magnitudes[first], c, d);
    if (first == last)
        return head * (end - start);

    const float middle = std::accumulate(magnitudes.begin() + first + 1, magnitudes.begin() + last, 0.0f);
    return head * (static_cast<float>(first + 1) - start) + middle + magnitudes[last] * (end - static_cast<float>(last));
}

} // namespace

SpectrumAnalyzer::SpectrumAnalyzer()
{
    std::ranges::for_each(std::views::iota(0, kFftSize), [this](int i) {
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * i / (kFftSize - 1)));
    });
}

const std::array<float, kBands + 1>& SpectrumAnalyzer::bandEdges()
{
    static const auto edges = [] {
        const float bla = 255.0f / fpow2(75.0f / 12.0f);
        std::array<float, kBands + 1> values {};
        std::ranges::for_each(std::views::iota(0, kBands + 1), [&](int x) {
            values[x] = (fpow2(static_cast<float>(x) / 12.0f) - 1.0f) * bla + 1.0f;
        });
        return values;
    }();
    return edges;
}

SpectrumFrame SpectrumAnalyzer::computeBands(std::array<float, kFftSize> windowed)
{
    forwardFft512(windowed);
    std::ranges::for_each(std::views::iota(0, kFftBins), [&windowed](int i) {
        const float sinT = windowed[2 * i];
        const float cosT = windowed[2 * i + 1];
        windowed[i] = std::sqrt(sinT * sinT + cosT * cosT) * kSpecScale;
    });

    const auto& edges = bandEdges();
    SpectrumFrame bands {};
    std::ranges::for_each(std::views::iota(0, kBands), [&](int x) {
        // The cast wraps like the original's char store.
        bands[x] = static_cast<std::uint8_t>(std::lrint(fastmin(bandValue(windowed, edges[x], edges[x + 1]), 255.0f)));
    });
    return bands;
}

SpectrumFrame SpectrumAnalyzer::takeFrame()
{
    std::array<float, kFftSize> windowed {};
    std::transform(buffer_.begin(), buffer_.end(), window_.begin(), windowed.begin(), std::multiplies<float>());
    std::copy(buffer_.begin() + kHopSize, buffer_.end(), buffer_.begin());
    used_ -= kHopSize;
    return computeBands(windowed);
}

} // namespace wcv
