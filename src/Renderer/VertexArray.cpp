#include "VertexArray.h"
#include <glad/glad.h>
#include <iostream>

VertexArray::VertexArray() {
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
    
    BufferLayout layout;
    layout.PushFloat(3); //position
    layout.PushFloat(3); //color
    layout.PushFloat(3); //normal
    layout.PushFloat(2); //uv
    layout.PushFloat(1); //specular

    Bind();
    vb.Bind();
    auto elems = layout.GetElements();
    unsigned int offset = 0;

    for (size_t i = 0; i < elems.size(); i++) {

        auto element = elems[i];
        auto stride = layout.GetStride();
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, 
            element.normalized? GL_TRUE: GL_FALSE, layout.GetStride(), (const void*)offset);
        offset += element.count * BufferLayoutElement::GetSizeOfType(element.type);
    }
};