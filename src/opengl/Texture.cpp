/*
Copyright (C) 2023 William Redding - All Rights Reserved
License: MIT
*/

#include <opengl/Texture.hpp>
#include <algorithm>
#include <glad/gl.h>
#include <stb_image.h>
#include <util/Log.hpp>

GLenum GetTextureFormat(int numComponents) {
    switch (numComponents) {
    case 1: {
        return GL_RED;
    }
    case 3: {
        return GL_RGB;
        break;
    }
    case 4: {
        return  GL_RGBA;
        break;
    }
    default: {
        return GL_INVALID_VALUE;
    }
    }
}

Texture::Texture(Texture&& other) noexcept : uID(other.uID)
{
    other.uID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        glDeleteTextures(1, &uID);
        uID = other.uID;
        other.uID = 0;
    }
    return *this;
}

Texture::Texture(std::string path)
{
    glGenTextures(1, &uID);
    glBindTexture(GL_TEXTURE_2D, uID);

    int width, height, numComponents;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &numComponents, 0);
    if (data) {
        GLenum format = GetTextureFormat(numComponents);
        if (format == GL_INVALID_VALUE) {
            format = GL_RGBA;
            LOG_WARNING("Invalid texture format for texture with {} channels at path: {}", numComponents, path);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else {
        LOG_ERROR("Failed to load texture at path {}", path);
    }
    stbi_image_free(data);
}


Texture::~Texture()
{
    glDeleteTextures(1, &uID);
}

void Texture::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, uID);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*
MIT License

Copyright (c) 2023 William Redding

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/