#pragma once

#include <buffers/index-buffer.h>
#include <graphics/renderer-2d.h>
#include <graphics/utils/check-ogl-errors.h>
#include <buffers/vertex-array.h>
#include <components/visual.h>
#include <components/physical.h>

namespace oni {
    namespace graphics {
        using namespace buffers;

        class BatchRenderer2D : public Renderer2D {

            unsigned long m_MaxSpriteCount;
            unsigned long m_MaxIndicesCount;

            unsigned long m_MaxBufferSize;
            GLsizei m_MaxVertexSize;
            GLsizei m_MaxSpriteSize;

            std::unique_ptr<IndexBuffer> m_IBO;

            // Actual number of indices used.
            GLsizei m_IndexCount;

            GLuint m_VDO;
            std::unique_ptr<VertexArray> m_VAO;

            // The buffer that will hold all the VertexData in the batch.
            components::VertexData *m_Buffer;

        public:
            explicit BatchRenderer2D(const unsigned long maxSpriteCount);

            ~BatchRenderer2D() { glDeleteBuffers(1, &m_VDO); };

            void begin() override;

            void submit(const components::Placement &position, const components::Appearance &appearance) override;

            void flush() override;

            void end() override;

        };

    }
}