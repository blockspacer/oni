#pragma once

#include <memory>

#include <GL/glew.h>

#include <oni-core/math/vec3.h>

namespace ftgl {
    class texture_atlas_t;

    class texture_font_t;

    class texture_glyph_t;
}

namespace oni {
    namespace components {
        class Text;
    }

    namespace graphics {
        class FontManager {

            // Wrap atlas and font with unique_ptr and pass the custom deleter
//            std::unique_ptr<ftgl::texture_font_t, decltype(&ftgl::texture_font_delete)> m_FTFont;
//            std::unique_ptr<ftgl::texture_atlas_t, decltype(&ftgl::texture_atlas_delete)> m_FTAtlas;
            ftgl::texture_atlas_t *m_FTAtlas;
            ftgl::texture_font_t *m_FTFont;

            float mGameWidth;
            float mGameHeight;

        public:
            FontManager(std::string font, int size, float gameWidth, float gameHeight);

            ~FontManager();

            FontManager(const FontManager &) = delete;

            FontManager &operator=(FontManager &) = delete;

            components::Text createTextFromString(const std::string &text, const math::vec3 &position);

            void updateText(const std::string &textContent, components::Text &text);

            size_t getAtlasWidth() const;

            size_t getAtlasHeight() const;

            unsigned char *getAtlasData() const;

            GLuint getTextureID() const;

        private:
            const ftgl::texture_glyph_t *findGlyph(const char &character) const;
        };
    }
}