#include "Fft.h"

#include <algorithm>
#include <complex>
#include <numbers>
#include <numeric>
#include <ranges>

namespace wcv {
namespace {

constexpr int kFftOrder = 9;

unsigned reverseBits(unsigned value)
{
    const auto bits = std::views::iota(0, kFftOrder);
    return std::accumulate(bits.begin(), bits.end(), 0u, [value](unsigned reversed, int bit) {
        return (reversed << 1) | ((value >> bit) & 1u);
    });
}

const std::array<std::complex<float>, kFftSize / 2>& twiddles()
{
    static const auto table = [] {
        std::array<std::complex<float>, kFftSize / 2> values {};
        std::ranges::for_each(std::views::iota(0, kFftSize / 2), [&values](int k) {
            values[k] = std::polar(1.0f, -2.0f * std::numbers::pi_v<float> * k / kFftSize);
        });
        return values;
    }();
    return table;
}

} // namespace

void forwardFft512(std::array<float, kFftSize>& signal)
{
    std::array<std::complex<float>, kFftSize> bins {};
    std::ranges::for_each(std::views::iota(0u, unsigned(kFftSize)), [&](unsigned i) {
        bins[reverseBits(i)] = signal[i];
    });

    std::ranges::for_each(std::views::iota(1, kFftOrder + 1), [&bins](int stage) {
        const int span = 1 << stage;
        const int half = span / 2;
        std::ranges::for_each(std::views::iota(0, kFftSize / 2), [&](int butterfly) {
            const int k = butterfly % half;
            const int top = (butterfly / half) * span + k;
            const auto even = bins[top];
            const auto odd = bins[top + half] * twiddles()[k * (kFftSize / span)];
            bins[top] = even + odd;
            bins[top + half] = even - odd;
        });
    });

    signal[0] = bins[0].real();
    signal[1] = bins[kFftBins].real();
    std::ranges::for_each(std::views::iota(1, kFftBins), [&](int bin) {
        signal[2 * bin] = bins[bin].real();
        signal[2 * bin + 1] = bins[bin].imag();
    });
}

} // namespace wcv
