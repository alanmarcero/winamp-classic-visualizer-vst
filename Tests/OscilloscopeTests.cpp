#include "Oscilloscope.h"
#include "TestSignals.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <ranges>

namespace {

bool allEqual(const wcv::ScopeFrame& frame, int value)
{
    return std::ranges::all_of(frame, [value](std::int8_t point) { return point == value; });
}

} // namespace

TEST_CASE("the scope keeps the top byte of each 16-bit sample, divided by 8")
{
    // 0.5 is PCM 16384, top byte 64, so 64 / 8 = 8.
    const auto positive = test::filled<wcv::kScopeBlock>(0.5f);
    const auto negative = test::filled<wcv::kScopeBlock>(-0.5f);

    CHECK(allEqual(wcv::Oscilloscope::computeFrame(positive, positive, 2), 8));
    CHECK(allEqual(wcv::Oscilloscope::computeFrame(negative, negative, 2), -8));
}

TEST_CASE("the scope averages the two channels")
{
    const auto left = test::filled<wcv::kScopeBlock>(0.5f);
    const auto right = test::filled<wcv::kScopeBlock>(0.0f);

    CHECK(allEqual(wcv::Oscilloscope::computeFrame(left, right, 2), 4));
}

TEST_CASE("a mono scope reads only the left channel")
{
    const auto left = test::filled<wcv::kScopeBlock>(0.5f);
    const auto right = test::filled<wcv::kScopeBlock>(-1.0f);

    CHECK(allEqual(wcv::Oscilloscope::computeFrame(left, right, 1), 8));
}

TEST_CASE("the scope takes the nearest sample at 576 / 75 steps")
{
    // Point 1 reads sample int(1 * 7.68) = 7, point 74 reads sample 568.
    std::array<float, wcv::kScopeBlock> ramp {};
    std::ranges::transform(std::views::iota(0, wcv::kScopeBlock), ramp.begin(),
                           [](int i) { return static_cast<float>(i) / 1024.0f; });

    const auto frame = wcv::Oscilloscope::computeFrame(ramp, ramp, 2);

    // Sample 7 is PCM 224, top byte 0. Sample 568 is PCM 18176, top byte 71, 71 / 8 = 8.875.
    CHECK(frame[1] == 0);
    CHECK(frame[74] == 9);
}

TEST_CASE("a frame comes every 576 samples")
{
    wcv::Oscilloscope scope;
    int frames = 0;
    const std::vector<float> silence(1152, 0.0f);

    scope.push(silence, silence, 2, [&frames](const wcv::ScopeFrame&) { ++frames; });

    CHECK(frames == 2);
}
