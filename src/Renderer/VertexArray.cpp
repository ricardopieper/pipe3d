#include "VertexArray.h"
#include <glad/glad.h>
#include <iostream>

VertexArray::VertexArray(BufferLayout bufferLayout): bufferLayout(bufferLayout) {
    glGenVertexArrays(1, &rendererId);
}

void VertexArray::Dispose() const {
    glDeleteVertexArrays(1, &rendererId);
};

void VertexArray::Bind() const {
    glBindVertexArray(rendererId);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}

void VertexArray::AddBufferAndBind(VertexBuffer& vb) {

    Bind();
    vb.Bind();
    auto elems = bufferLayout.GetElements();
    unsigned int offset = 0;

    for (size_t i = 0; i < elems.size(); i++) {

        auto element = elems[i];
        auto stride = bufferLayout.GetStride();
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, 
            element.normalized? GL_TRUE: GL_FALSE, bufferLayout.GetStride(), (const void*)offset);
        offset += element.count * BufferLayoutElement::GetSizeOfType(element.type);
    }
};