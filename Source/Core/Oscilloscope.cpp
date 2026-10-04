// Ported from makeOscData in Src/Winamp/classic_vis.cpp,
// (c) Nullsoft, Inc. Winamp Collaborative License 1.0: private use only.

#include "Oscilloscope.h"

#include <cmath>
#include <ranges>

namespace wcv {
namespace {

// Winamp reads only the most significant byte of each 16-bit sample.
int mostSignificantByte(float sample)
{
    const long pcm = std::clamp(std::lrint(sample * 32768.0f), -32768L, 32767L);
    return static_cast<int>(pcm >> 8);
}

} // namespace

ScopeFrame Oscilloscope::computeFrame(const std::array<float, kScopeBlock>& left,
                                      const std::array<float, kScopeBlock>& right, int channels)
{
    const float step = kScopeBlock / 75.0f;
    ScopeFrame frame {};
    std::ranges::for_each(std::views::iota(0, kBands), [&](int x) {
        const auto index = static_cast<std::size_t>(static_cast<float>(x) * step);
        const float leftValue = static_cast<float>(mostSignificantByte(left[index])) / 8.0f;
        if (channels == 1) {
            frame[x] = static_cast<std::int8_t>(std::lrint(leftValue));
            return;
        }
        const float rightValue = static_cast<float>(mostSignificantByte(right[index])) / 8.0f;
        frame[x] = static_cast<std::int8_t>(std::lrint((leftValue + rightValue) / 2.0f));
    });
    return frame;
}

} // namespace wcv
