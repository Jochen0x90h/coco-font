#pragma once

#include <coco/String.hpp>
#include <coco/Vector2.hpp>
#include <cstdint>
#ifdef NATIVE
#include <ostream>
#endif


namespace coco {


struct Font {
	struct NonPrintable {

		int width;
	};

	struct Glyph {
		// offset of glyph in data
		int offset;

		// Y-position of glyph
		int y;

		// size of glyph
		int2 size;
	};

	struct TextureGlyph {
		// position of glyph on the texture
		int2 position;

		// Y-position of glyph
		int y;

		// size of glyph
		int2 size;
	};

	struct GlyphInfo {
		// t (glyph text): 18 bit
		// w : 7 bit
		// h : 7 bit
		uint32_t x;

		// offset: 24 bit
		// y : 7 bit
		// flag (glyph code or offset): 1 bit
		uint32_t y;

		bool printable() const {
			return (y & 0xffffff) != 0xffffff;
		}

		int width() const {
			return (x >> 18) & 0x7f;
		}

		// get non-printable
		NonPrintable nonPrintable() const {
			return {int((x >> 18) & 0x7f)};
		}

		// get glyph
		Glyph glyph() const {
			return {
				int(y & 0xffffff),
				int((y >> 24) & 0x7f),
				{int((x >> 18) & 0x7f), int((x >> 25) & 0x7f)}
			};
		}

		// get glyph on a texture
		TextureGlyph textureGlyph() {
			return {
				{int(y & 0xfff), int((y >> 12) & 0xfff)},
				int((y >> 24) & 0x7f),
				{int((x >> 18) & 0x7f), int((x >> 25) & 0x7f)}
			};
		}
	};


	// size of gap between characters
	uint8_t gapWidth;

	// size of space character
	//uint8_t spaceWidth;

	// size of tabulator
	//uint8_t tabWidth;

	// overall character height
	uint8_t height;

	// glyph bitmap data
	const uint8_t *data;

	// size of bitmap data either 1D or 2D (width in lower 16 bit, height in upper 16 bit)
	const int dataSize;

	// begin of glyph list
	const GlyphInfo *begin;

	// end of glyph list
	const GlyphInfo *end;


	static const Glyph tabGlyph;
	static const Glyph spaceGlyph;



	struct GlyphRange {
		const Font &font;
		String text;

		struct Iterator {
			const Font &font;
			String text;
			const Font::GlyphInfo *info;

			Iterator &operator ++();

			bool operator ==(const Iterator &it) const {
				return this->info == it.info;
			}

			Font::GlyphInfo operator *() {
				return *this->info;
			}
		};

		Iterator begin() {
			Iterator it{this->font, text};
			++it;
			return it;
		}

		Iterator end() {
			return {this->font, String(), nullptr};
		}
	};

	GlyphRange glyphRange(String text) const {return {*this, text};}



	int calcWidth(String text) const;


// helpers

	struct Less {
		/**
		 * @return true if s < g.s (assuming s is not empty)
		 */
		bool operator ()(const String &s, const GlyphInfo &info);
	};

	/**
	 * @return length of text if s starts with glyph text (assuming s is not empty)
	 */
	static int startsWith(const String &s, const GlyphInfo &info);

	/**
	 * Skip first UTF-8 character
	 */
	static String removeFirst(String s);
};

/*
#ifdef NATIVE
inline std::ostream &operator <<(std::ostream &s, const Font::Glyph &g) {
	auto gs = g.s;
	int i = 0;
	char buf[4];
	while (gs > 0) {
		buf[i] = gs;
		gs >>= 8;
		++i;
	}
	s.write(buf, i);
	return s;
}
#endif
*/

} // namespace coco
