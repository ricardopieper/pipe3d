#pragma once

#include "VertexBuffer.h"
#include "BufferLayout.h"

class VertexArray {
private:
    unsigned int rendererId;
    BufferLayout bufferLayout;
public:
    VertexArray() = default;
    VertexArray(BufferLayout layout);
    void Dispose() const;
    void AddBufferAndBind(VertexBuffer& vb);
    void Bind() const;
    void Unbind() const;
};