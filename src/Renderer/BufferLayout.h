#pragma once

#include <vector>
#include <glad/glad.h>
#include <assert.h>

struct BufferLayoutElement {
    unsigned int count;
    unsigned int type;
    bool normalized;

    static unsigned int GetSizeOfType(unsigned int type) {
        switch (type) {
            case GL_FLOAT: return sizeof(GLfloat);
            case GL_UNSIGNED_INT: return sizeof(GLuint);
            case GL_UNSIGNED_BYTE: return sizeof(GLbyte);
        }
        assert(false);
        return 0;
    }

};

class BufferLayout {
private:
    std::vector<BufferLayoutElement> elements;
    unsigned int stride;
public:
    BufferLayout(): stride(0) {};

    void PushFloat(unsigned int count) {
        elements.push_back({count, GL_FLOAT, false});
        stride += sizeof(GLfloat) * count;
    }

    void PushUint(unsigned int count) {
        elements.push_back({count, GL_UNSIGNED_INT, false});
        stride += sizeof(GLuint) * count;
    }

    void PushByte(unsigned int count) {
        elements.push_back({count, GL_UNSIGNED_BYTE, true});
        stride += sizeof(GLbyte) * count;
    }

    inline unsigned int GetStride() const {
        return stride;
    }

    inline const std::vector<BufferLayoutElement>& GetElements() const {
        return elements;
    }
};
