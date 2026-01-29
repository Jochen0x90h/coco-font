#pragma once

#include <coco/convert.hpp>
#include <coco/String.hpp>
#include <coco/Vector2.hpp>
#include <cstdint>
#ifdef NATIVE
#include <ostream>
#endif


namespace coco {

struct GlyphInfo {
    // code: 18 bit
    // width : 7 bit
    // height : 7 bit
    uint32_t data1;

    // location: 24 bit
    // y : 7 bit
    // extended flag (code contains offset to extended data, not supported yet): 1 bit
    uint32_t data2;


    /// @brief Get extended flag.
    /// @return Extended flag
    bool extended() const {
        return data2 >> 31;
    }

    /// @brief Get code point of glyph (only valid unless extended).
    /// @return Code point
    int code() const {
        return data1 & 0x3ffff;
    }
};



/// @brief Compare string with glyph info by comparing the string with the glyph's text
/// @return true if s < g.s (assuming s is not empty)
bool operator <(const String &s, const GlyphInfo &info);

inline bool operator <(int code, const GlyphInfo &info) {
    return code < info.code();
}

inline bool operator <(const GlyphInfo &info, int code) {
    return info.code() < code;
}

/// @brief Check if string starts with glyph text
/// @return length of text if s starts with glyph text (assuming s is not empty)
int startsWith(const String &s, const GlyphInfo &info);

/// @brief Remove first UTF-8 character
/// @return String without first UTF-8 character
//String removeFirstUtf8(String s);


struct LinearFontTraits {
    using LocationType = int;

    static int getLocation(uint32_t data2) {
        return int(data2 & 0xffffff);
    }
};

struct TextureFontTraits {
    using LocationType = int2;

    static int2 getLocation(uint32_t data2) {
        return {int(data2 & 0xfff), int((data2 >> 12) & 0xfff)};
    }
};


/// @brief Font
/// @tparam T Font traits
template <typename T>
struct Font {
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

    // begin of glyph list (first glyph is a placeholder for unknown characters)
    const GlyphInfo *begin;

    // end of glyph list
    const GlyphInfo *end;


    //static const Glyph tabGlyph;
    //static const Glyph spaceGlyph;


    struct Glyph {
        // size of glyph
        int2 size;

        // y-position of glyph
        int y;

        // location in data (linear data or font texture)
        T::LocationType location;
    };


    struct GlyphRange {
        const Font &font;
        String text;

        struct Iterator {
            const Font<T> &font;
            String text;
            const GlyphInfo *info;

            Iterator &operator ++() {
                if (this->text.size() > 0) {
                    auto begin = this->font.begin + 2;
                    //Font::Less less;
                    auto info = std::upper_bound(begin, this->font.end, this->text);//, less);
                    --info;

                    int l = startsWith(this->text, *info);
                    if (l) {
                        // found character sequence
                        this->info = info;
                    } else {
                        // unknown character, use placeholder (first glyph)
                        this->info = this->font.begin;

                        // get length of first utf-8 character
                        l = std::max(utf8(this->text).length, 1);
                    }

                    // remove character sequence
                    this->text = this->text.substring(l);
                } else {
                    this->info = nullptr;
                }
                return *this;
            }

            bool operator ==(const Iterator &it) const {
                return this->info == it.info;
            }

            Glyph operator *() {
                auto data1 = this->info->data1;
                auto data2 = this->info->data2;
                return {
                    {int((data1 >> 18) & 0x7f), int(data1 >> 25)}, // size
                    int((data2 >> 24) & 0x7f), // y
                    T::getLocation(data2) // location
                };
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

    /// @brief Get glyph range for text that can be used to iterate over glyphs in range-based for loop
    /// Example:
    /// for (auto glyph : font.glyphRange(text)) {
    ///   if (glyph.size.y == 0) {
    ///     // non-printable, e.g. ' '
    ///   } else {
    ///     // print glyph
    ///   }
    /// }
    /// @param text Text to get glyphs for
    /// @return GlyphRange object that can be used in range-based for loop
    GlyphRange glyphRange(String text) const {return {*this, text};}



    int calcWidth(String text) const {
        int x = 0;
        for (auto info : glyphRange(text)) {
            // add glyph width
            x += info.size.x;

            // add space between characters
            x += this->gapWidth;
        }
        return x;
    }

    /// @brief Return the next code provided by the font.
    /// Includes only unicode code points
    /// @param code Code
    /// @param includePlaceholder If true, the placeholder with code 0 is included in the search
    /// @return Next code or first code if the last code was given
    int nextCode(int code, bool includePlaceholder = false) const {
        auto begin = this->begin + (includePlaceholder ? 0 : 1);
        auto end = this->end;
        auto info = std::upper_bound(begin, end, code);
        return info == end ? begin->code() : info->code();
    }

    /// @brief Return the previous code provided by the font.
    /// Includes only unicode code points
    /// @param code Code
    /// @param includePlaceholder If true, the placeholder with code 0 is included in the search
    /// @return Previous code or last code if the first code was given
    int prevCode(int code, bool includePlaceholder = false) const {
        auto begin = this->begin + (includePlaceholder ? 0 : 1);
        auto end = this->end;
        auto info = std::lower_bound(begin, end, code);
        --info;
        return info < begin ? (end - 1)->code() : info->code();
    }
};

using LinearFont = Font<LinearFontTraits>;
using TextureFont = Font<TextureFontTraits>;


/*
struct Font {
    //struct NonPrintable {

    //	int width;
    //};

    / *struct Glyph {
        // offset of glyph in linear glyph data
        int offset;

        // output Y-position of glyph
        int y;

        // size of glyph
        int2 size;
    };

    struct TextureGlyph {
        // position of glyph on the texture
        int2 position;

        // output Y-position of glyph
        int y;

        // size of glyph
        int2 size;
    };* /

    struct GlyphInfo {
        // code: 18 bit
        // width : 7 bit
        // height : 7 bit
        uint32_t data1;

        // offset: 24 bit
        // y : 7 bit
        // extended flag (code contains offset to extended data, not supported yet): 1 bit
        uint32_t data2;

        /// @brief Get printable property.
        /// @return True if glyph is printable
        bool printable() const {
            return (data2 & 0xffffff) != 0xffffff;
        }

        /// @brief Get code point of glyph (only valid unless extended).
        /// @return Code point
        int code() const {
            return data1 & 0x3ffff;
        }

        /// @brief Get glyph width.
        /// @return Glyph width
        int width() const {
            return (data1 >> 18) & 0x7f;
        }

        /// @brief Get glyph height.
        /// @return Glyph height
        int height() const {
            return data1 >> 25;
        }

        /// @brief Get size of glyph.
        /// @return Glyph size
        int2 size() const {
            return {int((data1 >> 18) & 0x7f), int(data1 >> 25)};
        }

        /// @brief Get offset in linear glyph data.
        /// @return Offset in glyph data
        int offset() const {
            return int(data2 & 0xffffff);
        }

        /// @brief Get position on glyph texture.
        /// @return Position on texture
        int2 position() const {
            return {int(data2 & 0xfff), int((data2 >> 12) & 0xfff)};
        }

        /// @brief Get Y-position of glyph on the output.
        /// Small glyphs such as '.' need to be placed vertically
        /// @return Y-position
        int y() const {
            return int((data2 >> 24) & 0x7f);
        }

        // get non-printable
        //NonPrintable nonPrintable() const {
        //	return {int((x >> 18) & 0x7f)};
        //}

        // get glyph
        / *Glyph glyph() const {
            return {
                int(data2 & 0xffffff),
                int((data2 >> 24) & 0x7f),
                {int((data1 >> 18) & 0x7f), int((data1 >> 25) & 0x7f)}
            };
        }

        // get glyph on a texture
        TextureGlyph textureGlyph() {
            return {
                {int(data2 & 0xfff), int((data2 >> 12) & 0xfff)},
                int((data2 >> 24) & 0x7f),
                {int((data1 >> 18) & 0x7f), int((data1 >> 25) & 0x7f)}
            };
        }* /
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


    //static const Glyph tabGlyph;
    //static const Glyph spaceGlyph;



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

    /// @brief Get glyph range for text that can be used to iterate over glyphs in range-based for loop
    /// Example:
    /// for (auto info : font.glyphRange(text)) {
    ///   if (!info.printable()) {
    ///     // special character
    ///   } else {
    ///     auto glyph = info.glyph();
    ///   }
    /// }
    /// @param text Text to get glyphs for
    /// @return GlyphRange object that can be used in range-based for loop
    GlyphRange glyphRange(String text) const {return {*this, text};}



    int calcWidth(String text) const;


// helpers

    struct Less {
        /// @brief Compare string with glyph info by comparing the string with the glyph's text
        /// @return true if s < g.s (assuming s is not empty)
        bool operator ()(const String &s, const GlyphInfo &info);
    };

    /// @brief Check if string starts with glyph text
    /// @return length of text if s starts with glyph text (assuming s is not empty)
    static int startsWith(const String &s, const GlyphInfo &info);

    /// @brief Skip first UTF-8 character
    ///
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
