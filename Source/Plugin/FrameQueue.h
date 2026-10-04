#pragma once

#include <juce_core/juce_core.h>

#include <array>
#include <optional>

// Lock-free hand-off of analysis frames from the audio thread to the editor.
template <typename Frame, int Capacity = 32>
class FrameQueue {
public:
    // Drops the frame when the queue is full, so the audio thread never waits.
    void push(const Frame& frame)
    {
        const auto scope = fifo_.write(1);
        if (scope.blockSize1 > 0)
            frames_[static_cast<std::size_t>(scope.startIndex1)] = frame;
    }

    // Empties the queue and returns the newest frame.
    std::optional<Frame> popLatest()
    {
        const int ready = fifo_.getNumReady();
        if (ready == 0)
            return std::nullopt;

        const auto scope = fifo_.read(ready);
        const int last = scope.blockSize2 > 0 ? scope.startIndex2 + scope.blockSize2 - 1
                                              : scope.startIndex1 + scope.blockSize1 - 1;
        return frames_[static_cast<std::size_t>(last)];
    }

private:
    juce::AbstractFifo fifo_ { Capacity };
    std::array<Frame, Capacity> frames_ {};
};
