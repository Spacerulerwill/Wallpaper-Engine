#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <any>
#include <gl.h>
#include <opengl/Window.hpp>
#include <string>
#include <unordered_map>

struct BuiltinUniformsLocationsStruct {
    GLint time = -1;
    GLint resolution = -1;
    GLint mousePos = -1;
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
};

#endif // !SHADER_MANAGER_H