#include "Fft.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <ranges>

using Catch::Approx;

TEST_CASE("an impulse gives a flat spectrum in Perm layout")
{
    std::array<float, wcv::kFftSize> signal {};
    signal[0] = 1.0f;

    wcv::forwardFft512(signal);

    CHECK(signal[0] == Approx(1.0f));   // Re0
    CHECK(signal[1] == Approx(1.0f));   // Re256
    CHECK(signal[2] == Approx(1.0f));   // Re1
    CHECK(signal[3] == Approx(0.0f).margin(1e-5));   // Im1
    CHECK(signal[200] == Approx(1.0f)); // Re100
    CHECK(signal[511] == Approx(0.0f).margin(1e-5)); // Im255
}

TEST_CASE("a cosine on bin 8 has unnormalised magnitude N / 2 on bin 8 only")
{
    std::array<float, wcv::kFftSize> signal {};
    std::ranges::transform(std::views::iota(0, wcv::kFftSize), signal.begin(), [](int i) {
        return static_cast<float>(std::cos(2.0 * std::numbers::pi * 8.0 * i / wcv::kFftSize));
    });

    wcv::forwardFft512(signal);

    CHECK(std::hypot(signal[16], signal[17]) == Approx(256.0f).epsilon(1e-4));
    CHECK(std::hypot(signal[14], signal[15]) == Approx(0.0f).margin(1e-3));
    CHECK(std::hypot(signal[18], signal[19]) == Approx(0.0f).margin(1e-3));
}
