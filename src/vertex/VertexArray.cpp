#include "VertexArray.hpp"
#include <gl.h>

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_ID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_ID);
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
    Bind();
    vb.Bind();
    std::vector<VertexBufferLayoutElement> elements = layout.GetElements();
    size_t offset = 0;
    for (int i = 0; i < elements.size(); i++) {
        const VertexBufferLayoutElement element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), reinterpret_cast<const void*>(offset));
        offset += element.count * VertexBufferLayoutElement::GetSize(element.type);
    }
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_ID);

}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}
