#pragma once
#include "glad/glad.h"
#include <vector>

struct VertexBufferLayoutElement {
	unsigned int type;
	unsigned int count;
    GLuint normalized;

    static unsigned int GetSize(unsigned int type) {
        switch (type)
        {
        case GL_FLOAT:
            return 4;
            break;
        case GL_UNSIGNED_INT:
            return 4;
            break;
        case GL_UNSIGNED_BYTE:
            return 1;
            break;
        }
        return 0;
    }
};

class VertexBufferLayout {
public:
    VertexBufferLayout();

    template <typename T>
    void AddAttribute(unsigned int count);

    std::vector<VertexBufferLayoutElement> GetElements() const;
    unsigned int GetStride() const;
private:
    unsigned int m_Stride;
    std::vector<VertexBufferLayoutElement> m_Elements;
};
