#pragma once

#include "VisConstants.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace wcv {

using ScopeFrame = std::array<std::int8_t, kBands>;

// Matches makeOscData (Src/Winamp/classic_vis.cpp). Winamp gives the vis
// 576 samples per block and takes 75 points from them.
class Oscilloscope {
public:
    // Calls onFrame once for each block of 576 samples.
    template <typename OnFrame>
    void push(std::span<const float> left, std::span<const float> right, int channels, OnFrame&& onFrame)
    {
        std::size_t offset = 0;
        while (offset < left.size()) {
            const std::size_t count = std::min(std::size_t(kScopeBlock) - used_, left.size() - offset);
            std::ranges::copy(left.subspan(offset, count), std::span(left_).subspan(used_).begin());
            std::ranges::copy(right.subspan(offset, count), std::span(right_).subspan(used_).begin());
            used_ += count;
            offset += count;
            if (used_ == kScopeBlock) {
                used_ = 0;
                onFrame(computeFrame(left_, right_, channels));
            }
        }
    }

    // channels is 1 or 2. With 1, only left is read.
    static ScopeFrame computeFrame(const std::array<float, kScopeBlock>& left,
                                   const std::array<float, kScopeBlock>& right, int channels);

private:
    std::array<float, kScopeBlock> left_ {};
    std::array<float, kScopeBlock> right_ {};
    std::size_t used_ = 0;
};

} // namespace wcv
