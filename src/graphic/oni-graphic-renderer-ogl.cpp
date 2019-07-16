#include <oni-core/graphic/oni-graphic-renderer-ogl.h>

#include <algorithm>
#include <cassert>

#include <GL/glew.h>

#include <oni-core/graphic/oni-graphic-shader.h>
#include <oni-core/graphic/oni-graphic-texture-manager.h>


namespace oni {
    namespace graphic {
        Renderer_OpenGL::Renderer_OpenGL(PrimitiveType primitiveType) : mPrimitiveType(primitiveType) {
            mFrameBuffer = std::make_unique<FrameBuffer>();
        }

        Renderer_OpenGL::~Renderer_OpenGL() = default;

        common::oniGLint
        Renderer_OpenGL::getSamplerID(common::oniGLuint textureID) {
            auto it = std::find(mTextures.begin(), mTextures.end(), textureID);
            if (it == mTextures.end()) {
                /*
                 * This can be used to render more than max number of samplers, but it slows things down.
                if (mNextSamplerID > mMaxNumTextureSamplers) {
                    reset();
                }*/
                assert(mNextSamplerID <= mMaxNumTextureSamplers);

                mTextures.push_back(textureID);
                mSamplers.push_back(mNextSamplerID++);
                return mSamplers.back();
            } else {
                auto index = std::distance(mTextures.begin(), it);
                return mSamplers[index];
            }
        }

        void
        Renderer_OpenGL::_begin(const math::mat4 &model,
                                const math::mat4 &view,
                                const math::mat4 &proj,
                                const math::vec2 &screenSize,
                                common::r32 zoom) {
            enableShader(model, view, proj, screenSize, zoom);

            mNextSamplerID = 0;
            mSamplers.clear();
            mTextures.clear();

            bindVertexBuffer();
            // Data written to mBuffer has to match the structure of VBO.
            mBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        }

        void
        Renderer_OpenGL::_end() {
            glUnmapBuffer(GL_ARRAY_BUFFER);
            unbindVertexBuffer();
        }

        void
        Renderer_OpenGL::_flush(component::Texture *renderTarget) {
            auto indexCount = getIndexCount();
            if (indexCount < 0) {
                return;
            }
            if (renderTarget) {
                bindFrameBuffer();
                attachFrameBuffer(*renderTarget);
            }

            TextureManager::bindRange(0, mTextures);

            bindVertexArray();
            bindIndexBuffer();

            switch (mPrimitiveType) {
                case PrimitiveType::POINTS: {
                    glDrawArrays(GL_POINTS, 0, indexCount);
                    break;
                }
                case PrimitiveType::LINES: {
                    glDrawArrays(GL_LINES, 0, indexCount);
                    break;
                }
                case PrimitiveType::TRIANGLES: {
                    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
                    break;
                }
                case PrimitiveType::TRIANGLE_STRIP: {
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, indexCount);
                    break;
                }
                default: {
                    assert(false);
                }
            }

            if (renderTarget) {
                TextureManager::loadFromTextureID(*renderTarget);
                unbindFrameBuffer();
            }
            unbindIndexBuffer();
            unbindVertexArray();

            TextureManager::unbind();

            resetIndexCount();

            disableShader();
        }

        void
        Renderer_OpenGL::disableShader() {
            mShader->disable();
        }

        void
        Renderer_OpenGL::bindVertexArray() {
            mVertexArray->bindVAO();
        }

        void
        Renderer_OpenGL::unbindVertexArray() {
            mVertexArray->unbindVAO();
        }

        void
        Renderer_OpenGL::bindVertexBuffer() {
            mVertexArray->bindVBO();
        }

        void
        Renderer_OpenGL::unbindVertexBuffer() {
            mVertexArray->unbindVBO();
        }

        void
        Renderer_OpenGL::bindIndexBuffer() {
            if (mIndexBuffer) {
                mIndexBuffer->bind();
            }
        }

        void
        Renderer_OpenGL::unbindIndexBuffer() {
            if (mIndexBuffer) {
                mIndexBuffer->unbind();
            }
        }

        void
        Renderer_OpenGL::bindFrameBuffer() {
            mFrameBuffer->bind();
        }

        void
        Renderer_OpenGL::unbindFrameBuffer() {
            mFrameBuffer->unbindAndClear();
        }

        void
        Renderer_OpenGL::attachFrameBuffer(component::Texture &renderTarget) {
            mFrameBuffer->attach(renderTarget);
        }

        void
        Renderer_OpenGL::setViewportSize(const Renderer::WindowSize &size) {
            glViewport(0, 0, size.width, size.height);
        }

        Renderer::WindowSize
        Renderer_OpenGL::getViewportSize() {
            // TODO: Only Window knows the correct value, have to figure out a better design for this.
            return {1600, 900};
        }
    }
}