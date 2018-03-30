#include <oni-core/graphics/texture.h>
#include <oni-core/graphics/utils/check-ogl-errors.h>

#include <FreeImage.h>

namespace oni {
    namespace graphics {
        components::Texture LoadTexture::load(const std::string &path) {
            FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
            FIBITMAP *dib = nullptr;

            fif = FreeImage_GetFileType(path.c_str(), 0);
            if (fif == FIF_UNKNOWN)
                fif = FreeImage_GetFIFFromFilename(path.c_str());
            if (fif == FIF_UNKNOWN) {
                std::runtime_error("Could not determine image type: " + path);
            }

            if (FreeImage_FIFSupportsReading(fif))
                dib = FreeImage_Load(fif, path.c_str());
            if (!dib) {
                std::runtime_error("Could load image: " + path);
            }

            auto bits = FreeImage_GetBits(dib);
            auto width = FreeImage_GetWidth(dib);
            auto height = FreeImage_GetHeight(dib);
            if ((bits == nullptr) || (width == 0) || (height == 0)) {
                std::runtime_error("Image loaded with no data: " + path);
            }

            GLuint textureID = 0;
            glGenTextures(1, &textureID);

            if(!textureID){
                throw std::runtime_error("Could not generate texture.");
            }

            bind(textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);

            unbind();

            FreeImage_Unload(dib);

            std::vector<math::vec2> uv{math::vec2(0, 0), math::vec2(0, 1), math::vec2(1, 1), math::vec2(1, 0)};

            return components::Texture(path, textureID, width, height, uv);
        }

        void LoadTexture::bind(GLuint textureID) {
            glBindTexture(GL_TEXTURE_2D, textureID);
        }

        void LoadTexture::unbind() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        GLuint LoadTexture::load(const graphics::FontManager &fontManager) {
            auto width = fontManager.getAtlasWidth();
            auto height = fontManager.getAtlasHeight();

            GLuint textureID = 0;
            glGenTextures(1, &textureID);

            if(!textureID){
                throw std::runtime_error("Could not generate texture.");
            }

            bind(textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE,
                         fontManager.getAtlasData());

            unbind();

            return textureID;
        }
    }
}