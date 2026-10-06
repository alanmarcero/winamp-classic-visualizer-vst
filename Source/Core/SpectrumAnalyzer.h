#pragma once

#include "VisConstants.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <span>

namespace wcv {

using SpectrumFrame = std::array<std::uint8_t, kBands>;

// Matches SABuffer (Src/Winamp/SABuffer.cpp) and makeSpecData
// (Src/Winamp/classic_vis.cpp). Sums L and R into a 512-sample window,
// moves the window 256 samples per frame, and gives 75 bands of 0 to 255.
class SpectrumAnalyzer {
public:
    SpectrumAnalyzer();

    // Calls onFrame once for each full window. Samples are in [-1, 1].
    template <typename OnFrame>
    void push(std::span<const float> left, std::span<const float> right, OnFrame&& onFrame)
    {
        std::size_t offset = 0;
        while (offset < left.size()) {
            const std::size_t count = std::min(std::size_t(kFftSize) - used_, left.size() - offset);
            const auto leftChunk = left.subspan(offset, count);
            std::transform(leftChunk.begin(), leftChunk.end(), right.subspan(offset, count).begin(),
                           std::span(buffer_).subspan(used_).begin(), std::plus<float>());
            used_ += count;
            offset += count;
            if (used_ == kFftSize)
                onFrame(takeFrame());
        }
    }

    // makeSpecData: FFT, magnitude, and the 75 semitone-spaced bands.
    static SpectrumFrame computeBands(std::array<float, kFftSize> windowed);

    // Fractional FFT bin where each band starts. Entry 75 is where the last band ends.
    static const std::array<float, kBands + 1>& bandEdges();

private:
    SpectrumFrame takeFrame();

    std::array<float, kFftSize> buffer_ {};
    std::array<float, kFftSize> window_ {};
    std::size_t used_ = 0;
};

} // namespace wcv
