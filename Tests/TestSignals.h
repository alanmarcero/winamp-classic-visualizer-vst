#pragma once

#include <array>
#include <cmath>
#include <numbers>
#include <ranges>
#include <vector>

namespace test {

inline std::vector<float> sine(double frequency, double sampleRate, std::size_t count, float amplitude = 1.0f)
{
    std::vector<float> samples(count);
    std::ranges::transform(std::views::iota(std::size_t { 0 }, count), samples.begin(), [&](std::size_t i) {
        return amplitude * static_cast<float>(std::sin(2.0 * std::numbers::pi * frequency * static_cast<double>(i) / sampleRate));
    });
    return samples;
}

template <std::size_t Size>
std::array<float, Size> filled(float value)
{
    std::array<float, Size> values {};
    values.fill(value);
    return values;
}

} // namespace test
