#include "Palette.h"

#include <algorithm>
#include <numeric>
#include <optional>
#include <ranges>

namespace wcv {
namespace {

// Winamp reads with fgets(buf, 90, fp), so one read takes at most 89 chars.
constexpr std::size_t kReadLimit = 89;

// One fgets call. Empty when the read hits end of file, because Winamp
// stops at feof() before it parses that read.
std::optional<std::string_view> nextRead(std::string_view text, std::size_t& position)
{
    const std::string_view remaining = text.substr(position);
    const std::size_t available = std::min(remaining.size(), kReadLimit);
    const std::size_t newline = remaining.substr(0, available).find('\n');
    if (newline != std::string_view::npos) {
        position += newline + 1;
        return remaining.substr(0, newline);
    }
    if (available < kReadLimit)
        return std::nullopt;

    position += kReadLimit;
    return remaining.substr(0, kReadLimit);
}

// Sets each component it can read. False when a component has no digits,
// which makes Winamp stop reading the file.
bool readColor(std::string_view line, Rgb& color)
{
    std::size_t cursor = 0;
    return std::ranges::all_of(std::views::iota(0, 3), [&](int component) {
        cursor = std::min(line.find_first_not_of(" ,\t", cursor), line.size());
        const std::size_t digitsEnd = std::min(line.find_first_not_of("0123456789", cursor), line.size());
        if (digitsEnd == cursor)
            return false;

        const auto digits = line.substr(cursor, digitsEnd - cursor);
        color[component] = static_cast<std::uint8_t>(std::accumulate(digits.begin(), digits.end(), 0u, [](unsigned value, char digit) {
            return value * 10 + static_cast<unsigned>(digit - '0');
        }));
        cursor = digitsEnd;
        return true;
    });
}

} // namespace

Palette parseViscolor(std::string_view text)
{
    Palette palette = kDefaultPalette;
    std::size_t position = 0;
    // all_of stops at the first read that fails, like Winamp's x = 24.
    static_cast<void>(std::ranges::all_of(palette, [&](Rgb& color) {
        const auto line = nextRead(text, position);
        return line.has_value() && readColor(*line, color);
    }));
    return palette;
}

} // namespace wcv
