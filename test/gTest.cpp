#include <gtest/gtest.h>
//#include "font/tahoma16pt8bpp.hpp"
#include <coco/Font.hpp>
#include <ranges>

using namespace coco;



// test code for glyph lookup principle
// ------------------------------------

struct TestGlyphInfo {
    String str;
};

bool operator <(const String &s, const TestGlyphInfo &g) {
    return s < g.str;
}

const TestGlyphInfo testGlyphs[] = {
    {"a"},
    {"b"},
    {"c"},
    {"f"},
    {"ft"},
    {"g"},
    {"ä"},
    {"ö"},
};

void drawText(String text) {
    while (text.size() > 0) {
        // find the glyph with the longest match, e.g. if text is "ftg" then glyph "ft" should be found
        auto begin = std::begin(testGlyphs) + 1;
        auto glyph = std::upper_bound(begin, std::end(testGlyphs), text);
        --glyph;

        if (text.startsWith(glyph->str)) {
            text = text.substring(glyph->str.size());

            // drawing the glyph would come here
        } else {
            // unknown character

            // remove first utf-8 character
            text = text.substring(utf8(text).length);
        }
    }
}

TEST(cocoTest, drawText) {
    drawText("aftgäüx");
}



/*TEST(cocoTest, font) {
    tahoma16pt8bpp.calcWidth("Ω");
}*/

// test code for Font.hpp
// ----------------------

// list of glyph infos for testing (without bitmap data)
static const GlyphInfo glyphs[] = {
    // {code, clocation}
    {0, 0}, // placeholder
    {32, 0}, // ' ' (non-printable)
    {65, 65}, // 'A'
    {66, 66}, // 'B'
    {0xD6, 0xD6}, // 'Ö'
    {0x2EB7, 0x2EB7}, // ⺷
    {0x1F60A, 0x1F60A}, // 😊
};

static const LinearFont font = {
    1, // gapWdith
    10, // height
    nullptr, // bitmap data
    0, // bitmap data size
    std::begin(glyphs),
    std::end(glyphs)
};

// test text, first character 'X' is not in font
String text = "X ABÖ⺷😊";

TEST(cocoTest, GlyphIterator) {
    //for (char ch : text) {std::cout << std::hex << int(uint8_t(ch)) << ' ';} std::cout << std::endl;

    const int expected[] = {0, 0, 65, 66, 0xD6, 0x2EB7, 0x1F60A};
    int i = 0;
    for (auto glyph : font.glyphRange(text)) {
        std::cout << glyph.location << std::endl;

        // check for expected glyph
        EXPECT_EQ(glyph.location, expected[i]);

        ++i;
    }
}

TEST(cocoTest, calcWidth) {
    int w = font.calcWidth(text);

    EXPECT_EQ(w, 7 * font.gapWidth);
}

TEST(cocoTest, nextCode) {
    EXPECT_EQ(font.nextCode(0), 32);
    EXPECT_EQ(font.nextCode(32), 65);
    EXPECT_EQ(font.nextCode(65), 66);
    EXPECT_EQ(font.nextCode(0x1F60A), 32);
    EXPECT_EQ(font.nextCode(0xfffff), 32);
}

TEST(cocoTest, prevCode) {
    EXPECT_EQ(font.prevCode(0), 0x1F60A);
    EXPECT_EQ(font.prevCode(32), 0x1F60A);
    EXPECT_EQ(font.prevCode(65), 32);
    EXPECT_EQ(font.prevCode(66), 65);
    EXPECT_EQ(font.prevCode(0x1F60A), 0x2EB7);
    EXPECT_EQ(font.prevCode(0xfffff), 0x1F60A);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    int success = RUN_ALL_TESTS();
    return success;
}
