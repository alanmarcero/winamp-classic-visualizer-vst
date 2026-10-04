#include "Palette.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("an empty viscolor.txt keeps the built-in colours")
{
    CHECK(wcv::parseViscolor("") == wcv::kDefaultPalette);
}

TEST_CASE("viscolor.txt sets colours in order and ignores trailing comments")
{
    const auto palette = wcv::parseViscolor("1,2,3, // background\n4, 5, 6\n\t7,8,9\r\n");

    CHECK(palette[0] == wcv::Rgb { 1, 2, 3 });
    CHECK(palette[1] == wcv::Rgb { 4, 5, 6 });
    CHECK(palette[2] == wcv::Rgb { 7, 8, 9 });
    CHECK(palette[3] == wcv::kDefaultPalette[3]);
}

TEST_CASE("a last line without a newline is dropped, as fgets hits end of file")
{
    const auto palette = wcv::parseViscolor("1,2,3\n4,5,6");

    CHECK(palette[0] == wcv::Rgb { 1, 2, 3 });
    CHECK(palette[1] == wcv::kDefaultPalette[1]);
}

TEST_CASE("a line without a number stops the read, after the numbers before it")
{
    const auto palette = wcv::parseViscolor("1,2\n4,5,6\n");

    CHECK(palette[0] == wcv::Rgb { 1, 2, wcv::kDefaultPalette[0][2] });
    CHECK(palette[1] == wcv::kDefaultPalette[1]);
}

TEST_CASE("values over 255 wrap like the original's unsigned char store")
{
    CHECK(wcv::parseViscolor("256,300,255\n")[0] == wcv::Rgb { 0, 44, 255 });
}
