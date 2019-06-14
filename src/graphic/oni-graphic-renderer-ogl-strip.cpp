#include <oni-core/graphic/oni-graphic-renderer-ogl-strip.h>

#include <GL/glew.h>

#include <oni-core/graphic/buffer/oni-graphic-buffer.h>
#include <oni-core/graphic/buffer/oni-graphic-index-buffer.h>
#include <oni-core/graphic/buffer/oni-graphic-vertex-array.h>
#include <oni-core/graphic/buffer/oni-graphic-buffer-data.h>
#include <oni-core/graphic/oni-graphic-shader.h>
#include <oni-core/graphic/oni-graphic-texture-manager.h>


namespace oni {
    namespace graphic {
        Renderer_OpenGL_Strip::Renderer_OpenGL_Strip(common::oniGLsizei maxSpriteCount) :
                Renderer_OpenGL(PrimitiveType::TRIANGLE_STRIP),
                mMaxPrimitiveCount(maxSpriteCount) {
            mVertexSize = sizeof(graphic::StripVertex);
            mMaxIndicesCount = mMaxPrimitiveCount;
            mPrimitiveSize = mVertexSize * 1;

            assert(mMaxIndicesCount < std::numeric_limits<common::i32>::max());

            common::oniGLsizei maxBufferSize{mPrimitiveSize * mMaxPrimitiveCount};

            auto vertShader = std::string_view("resources/shaders/strip.vert");
            auto geomShader = std::string_view("");
            auto fragShader = std::string_view("resources/shaders/strip.frag");
            mShader = std::make_unique<graphic::Shader>(vertShader,
                                                        geomShader,
                                                        fragShader);

            auto program = mShader->getProgram();

            auto last_I = glGetAttribLocation(program, "last");
            auto current_I = glGetAttribLocation(program, "current");
            auto next_I = glGetAttribLocation(program, "next");
            auto bc_I = glGetAttribLocation(program, "bc");
            auto texoff_I = glGetAttribLocation(program, "texoff");

            auto color_I = 1;//glGetAttribLocation(program, "color");
            auto samplerID_I = 1;//glGetAttribLocation(program, "samplerID");

            if (last_I == -1 || current_I == -1 || next_I == -1 || texoff_I == -1 ||
                color_I == -1 || samplerID_I == -1 || bc_I == -1) {
                assert(false);
            }

            common::oniGLsizei stride = mVertexSize;

            graphic::BufferStructure last;
            last.index = static_cast<common::oniGLuint>(last_I);
            last.componentCount = 4;
            last.componentType = GL_FLOAT;
            last.normalized = GL_FALSE;
            last.stride = stride;
            last.offset = nullptr;

            graphic::BufferStructure current;
            current.index = static_cast<common::oniGLuint>(current_I);
            current.componentCount = 4;
            current.componentType = GL_FLOAT;
            current.normalized = GL_FALSE;
            current.stride = stride;
            current.offset = reinterpret_cast<const common::oniGLvoid *>(stride * 2);

            graphic::BufferStructure texoff;
            texoff.index = static_cast<common::oniGLuint>(texoff_I);
            texoff.componentCount = 1;
            texoff.componentType = GL_FLOAT;
            texoff.normalized = GL_FALSE;
            texoff.stride = stride;
            texoff.offset = reinterpret_cast<const common::oniGLvoid *>(stride * 2 + 4);

            graphic::BufferStructure bc;
            bc.index = static_cast<common::oniGLuint>(bc_I);
            bc.componentCount = 3;
            bc.componentType = GL_FLOAT;
            bc.normalized = GL_FALSE;
            bc.stride = stride;
            bc.offset = reinterpret_cast<const common::oniGLvoid *>(stride * 2 + 5);

            graphic::BufferStructure next;
            next.index = static_cast<common::oniGLuint>(next_I);
            next.componentCount = 4;
            next.componentType = GL_FLOAT;
            next.normalized = GL_FALSE;
            next.stride = stride;
            next.offset = reinterpret_cast<const common::oniGLvoid *>(stride * 4);

//            graphic::BufferStructure color;
//            color.index = static_cast<common::oniGLuint>(color_I);
//            color.componentCount = 4;
//            color.componentType = GL_FLOAT;
//            color.normalized = GL_TRUE;
//            color.stride = stride;
//            color.offset = reinterpret_cast<const common::oniGLvoid *>(stride * 2 + 4);
//
//            graphic::BufferStructure samplerID;
//            samplerID.index = static_cast<common::oniGLuint>(samplerID_I);
//            samplerID.componentCount = 1;
//            samplerID.componentType = GL_FLOAT;
//            samplerID.normalized = GL_TRUE;
//            samplerID.stride = stride;
//            samplerID.offset = reinterpret_cast<const common::oniGLvoid *>(stride * 2 + 4);

            std::vector<graphic::BufferStructure> bufferStructures;
            bufferStructures.push_back(last);
            bufferStructures.push_back(current);
            bufferStructures.push_back(texoff);
            bufferStructures.push_back(bc);
            bufferStructures.push_back(next);
            //bufferStructures.push_back(color);
            //bufferStructures.push_back(samplerID);

            mVertexArray = std::make_unique<buffer::VertexArray>(bufferStructures, maxBufferSize);

            if (mMaxIndicesCount > mMaxPrimitiveCount) {
                mIndexBuffer = std::make_unique<buffer::IndexBuffer>(mMaxIndicesCount);
            }

            std::vector<GLint> samplers;
            for (common::i8 i = 0; i < mMaxNumTextureSamplers; ++i) {
                samplers.push_back(i);
            }

            mShader->enable();
            mShader->setUniformiv("samplers", samplers);
            mShader->disable();
        }

        Renderer_OpenGL_Strip::~Renderer_OpenGL_Strip() = default;

        void
        Renderer_OpenGL_Strip::submit(const component::WorldP4D &pos,
                                      const component::Appearance &appearance,
                                      const component::Texture &texture) {
            assert(mIndexCount + 1 < mMaxIndicesCount);

            auto buffer = static_cast<graphic::StripVertex *>(mBuffer);

            common::i32 samplerID = -1;
            if (!texture.image.path.empty()) {
                samplerID = getSamplerID(texture.textureID);
            }

            buffer->center = pos.value;
            buffer->texoff = 0.1f;
            buffer->bc = {1, 1, 1};
            buffer++;

            mBuffer = static_cast<void *>(buffer);

            mIndexCount += 1;
        }

        void
        Renderer_OpenGL_Strip::enableShader(const math::mat4 &model,
                                            const math::mat4 &view,
                                            const math::mat4 &proj) {
            mShader->enable();
            mShader->setUniformMat4("view", view);
            mShader->setUniformMat4("proj", proj);
        }

        void
        Renderer_OpenGL_Strip::disableShader() {
            mShader->disable();
        }

        void
        Renderer_OpenGL_Strip::bindVertexArray() {
            mVertexArray->bindVAO();
        }

        void
        Renderer_OpenGL_Strip::unbindVertexArray() {
            mVertexArray->unbindVAO();
        }

        void
        Renderer_OpenGL_Strip::bindVertexBuffer() {
            mVertexArray->bindVBO();
        }

        void
        Renderer_OpenGL_Strip::unbindVertexBuffer() {
            mVertexArray->unbindVBO();
        }

        void
        Renderer_OpenGL_Strip::bindIndexBuffer() {
            if (mIndexBuffer) {
                mIndexBuffer->bind();
            }
        }

        void
        Renderer_OpenGL_Strip::unbindIndexBuffer() {
            if (mIndexBuffer) {
                mIndexBuffer->unbind();
            }
        }

        common::oniGLsizei
        Renderer_OpenGL_Strip::getIndexCount() {
            return mIndexCount;
        }

        void
        Renderer_OpenGL_Strip::resetIndexCount() {
            mIndexCount = 0;
        }
    }
}