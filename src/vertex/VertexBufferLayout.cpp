#include "VertexBufferLayout.hpp"

VertexBufferLayout::VertexBufferLayout() :m_Stride(0)
{
}

template <>
void VertexBufferLayout::AddAttribute<float>(unsigned int count)
{
    m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
    m_Stride += VertexBufferLayoutElement::GetSize(GL_FLOAT) * count;
}

template <>
void VertexBufferLayout::AddAttribute<unsigned int>(unsigned int count)
{
    m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
    m_Stride += VertexBufferLayoutElement::GetSize(GL_UNSIGNED_INT) * count;
}

std::vector<VertexBufferLayoutElement> VertexBufferLayout::GetElements() const
{
    return m_Elements;
}

unsigned int VertexBufferLayout::GetStride() const
{
    return m_Stride;
}
