#pragma once

#include <any>
#include <fstream>
#include <gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <unordered_map>

struct Uniform {
    unsigned int location;
    GLenum type;
    void* var;
};

class ShaderManager
{
private:
    unsigned int u_ShaderProgramID = 0;

    const char* m_VertexShaderPath = "res/default.vert";
    std::string m_FragmentShaderPath = "";

    std::string ParseShader(const std::string& filepath);
    unsigned int CompileShader(unsigned int type, std::string& source);
    unsigned int CreateShader(std::string& vertex_source, std::string& fragmement_source);

    std::unordered_map<std::string, Uniform> m_UniformMap;
    void ClearUniformMap();

public:
    ShaderManager();
    ~ShaderManager();

    std::unordered_map<std::string, Uniform> getUniformMap() const;
    std::string getFragmentPath() const;

    void Bind() const;
    void Unbind() const;
    void SetShader(const std::string& fragment_path);

    int m_TimeUniformLoc = -1;
    int m_MouseUniformLoc = -1;
};

