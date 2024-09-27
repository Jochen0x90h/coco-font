#include "Font.hpp"


namespace coco {


Font::GlyphRange::Iterator &Font::GlyphRange::Iterator::operator ++() {
	if (this->text.size() > 0) {
		auto begin = this->font.begin + 2;
		Font::Less less;
		auto info = std::upper_bound(begin, this->font.end, this->text, less);
		--info;

		int l = Font::startsWith(this->text, *info);
		if (l) {
			// found character sequence
			this->info = info;

			// remove character sequence
			this->text = this->text.substring(l);
		} else {
			// unknown character, use placeholder
			this->info = this->font.begin;

			// remove first utf-8 character
			this->text = Font::removeFirst(this->text);
		}
	} else {
		this->info = nullptr;
	}
	return *this;
}

int Font::calcWidth(String text) const {
	int x = 0;
	for (auto info : glyphRange(text)) {
		// add glyph width
		x += info.width();

		// add space between characters
		x += this->gapWidth;
	}

	return x;
}

bool Font::Less::operator ()(const String &text, const GlyphInfo &info) {
	//return s < String(g.s);

	auto d = (const uint8_t*)text.data();
	int len = text.size();
	int code = info.x & 0x3ffff;

	// https://en.wikipedia.org/wiki/UTF-8

	if (code <= 0x7F) {
		return d[0] < code;
	}
	if (code <= 0x7FF) {
		int c0 = 0xC0 | (code >> 6);
		if (d[0] != c0)
			return d[0] < c0;
		if (len < 2)
			return false;
		return d[1] < (0x80 | (code & 0x3F));
	}
	if (code <= 0xFFFF) {
		int c0 = 0xE0 | (code >> 12);
		if (d[0] != c0)
			return d[0] < c0;
		if (len < 2)
			return false;
		int c1 = 0x80 | ((code >> 6) & 0x3F);
		if (d[1] != c1)
			return d[1] < c1;
		if (len < 3)
			return false;
		return d[2] < (0x80 | (code & 0x3F));
	}
	int c0 = 0xF0 | (code >> 18);
	if (d[0] != c0)
		return d[0] < c0;
	if (len < 2)
		return false;
	int c1 = 0x80 | ((code >> 12) & 0x3F);
	if (d[1] != c1)
		return d[1] < c1;
	if (len < 3)
		return false;
	int c2 = 0x80 | ((code >> 6) & 0x3F);
	if (d[2] != c2)
		return d[2] < c2;
	if (len < 4)
		return false;
	return d[3] < (0x80 | (code & 0x3F));
}

int Font::startsWith(const String &text, const GlyphInfo &info) {
	auto d = (const uint8_t*)text.data();
	int len = text.size();
	int code = info.x & 0x3ffff;

	if (code <= 0x7F) {
		return d[0] == code ? 1 : 0;
	}
	if (code <= 0x7FF) {
		if (len < 2)
			return 0;
		return d[0] == (0xC0 | (code >> 6)) && d[1] == (0x80 | (code & 0x3F)) ? 2 : 0;
	}
	if (code <= 0xFFFF) {
		if (len < 3)
			return 0;
		return d[0] == (0xE0 | (code >> 12)) && d[1] == (0x80 | ((code >> 6) & 0x3F)) && d[2] == (0x80 | (code & 0x3F)) ? 3 : 0;
	}
	if (len < 4)
		return 0;
	return d[0] == (0xF0 | (code >> 18)) && d[1] == (0x80 | ((code >> 12) & 0x3F)) && d[2] == (0x80 | ((code >> 6) & 0x3F)) && d[3] == (0x80 | (code & 0x3F)) ? 4 : 0;
}

String Font::removeFirst(String s) {
	// skip utf-8 character
	int start = s[0];
	int skip = 1;
	if (start & 0x80) {
		while (start & 0x40) {
			start <<= 1;
			++skip;
		}
	}
	return String(s.data() + skip, s.size() - skip);
}

} // namespace coco
