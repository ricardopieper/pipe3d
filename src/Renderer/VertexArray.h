#pragma once

#include "VertexBuffer.h"
#include "BufferLayout.h"

class VertexArray {
private:
    unsigned int rendererId;
public:
    VertexArray();
    void Dispose() const;
    void AddBufferAndBind(VertexBuffer& vb, BufferLayout& layout);
    void Bind() const;
    void Unbind() const;
};