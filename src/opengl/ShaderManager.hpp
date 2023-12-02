#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <any>
#include <gl.h>
#include <unordered_map>
#include <opengl/Window.hpp>
#include <string>
#include <unordered_map>

struct BuiltinUniformsLocationsStruct {
    GLint time = GL_INVALID_INDEX;
    GLint mousePos = GL_INVALID_INDEX;
};

struct Uniform {
    GLint location;
    GLenum type;
    std::any var;
};

class ShaderManager
{
private:
    GLuint uShaderProgramID = 0;
    GLuint uVertexShader = 0;
    bool LoadShader(GLenum shaderType, GLuint* shader, const std::string& path);
    bool ParseShader(const std::string& filepath, std::string* out);
    bool CompileShader(GLenum type, const std::string& source, GLuint* shaderIn);
public:
    ShaderManager();
    ~ShaderManager();
    bool SetFragmentShader(const std::string& fragmentPath, WindowDimensions windowDimensions);
    BuiltinUniformsLocationsStruct mBuiltinUniformsLocations;
    std::unordered_map<std::string, Uniform> mUniforms;
};

#endif // !SHADER_MANAGER_H