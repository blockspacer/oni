#pragma once

#include <stdexcept>
#include <vector>

#include <GL/glew.h>

#include <oni-core/components/visual.h>
#include <oni-core/components/buffer.h>
#include <oni-core/common/typedefs.h>

namespace oni {
    namespace buffers {

        /*
         * Great tutorial on OpenGL buffers: https://open.gl/drawing
         */
        class Buffer {
            GLuint mBufferID;
            common::BufferStructures mBufferStructures;

        public:
            Buffer(const std::vector<GLfloat> &data, GLsizeiptr dataSize, GLenum usage,
                   common::BufferStructures bufferStructures);

            ~Buffer() { glDeleteBuffers(1, &mBufferID); }

            Buffer &operator=(const Buffer &) = delete;

            Buffer &operator=(Buffer &) = delete;

            inline void bind() const { glBindBuffer(GL_ARRAY_BUFFER, mBufferID); }

            inline void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

            inline const auto getBufferStructure() const { return &mBufferStructures; }
        };

    }
}
