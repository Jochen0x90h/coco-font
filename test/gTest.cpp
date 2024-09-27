#include <gtest/gtest.h>
//#include "font/tahoma16pt8bpp.hpp"
#include <coco/Font.hpp>
#include <ranges>

using namespace coco;


/*TEST(cocoTest, font) {
	tahoma16pt8bpp.calcWidth("Ω");
}*/


static const Font::GlyphInfo glyphs[] = {
	{0, 0}, // placeholder
	{32, 0xffffff}, // ' '
	{65, 65}, // 'A'
	{66, 66}, // 'B'
	{0xD6, 0xD6}, // 'Ö'
	{0x2EB7, 0x2EB7}, // ⺷
	{0x1F60A, 0x1F60A}, // 😊
};

static const Font font = {
	1, // gapWdith
	10, // height
	nullptr, // bitmap data
	0, // bitmap data size
	std::begin(glyphs),
	std::end(glyphs)
};

String text = "X ABÖ⺷😊";

TEST(cocoTest, GlyphIterator) {
	//for (char ch : text) {std::cout << std::hex << int(uint8_t(ch)) << ' ';} std::cout << std::endl;

	const int expected[] = {0, 0xffffff, 65, 66, 0xD6, 0x2EB7, 0x1F60A};
	int i = 0;
	for (auto info : font.glyphRange(text)) {
		if (!info.printable()) {
			// non-printable
			auto nonPrintable = info.nonPrintable();
			std::cout << "non-printable " << int(nonPrintable.width) << std::endl;

			// check for expected glyph
			EXPECT_EQ(0xffffff, expected[i]);
		} else {
			// glyph
			auto glyph = info.glyph();
			std::cout << glyph.offset << std::endl;

			// check for expected glyph
			EXPECT_EQ(glyph.offset, expected[i]);
		}

		++i;
	}
}

TEST(cocoTest, calcWidth) {
	int w = font.calcWidth(text);

	EXPECT_EQ(w, 7 * font.gapWidth);
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	int success = RUN_ALL_TESTS();
	return success;
}
