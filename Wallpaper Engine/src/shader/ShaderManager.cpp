#include "ShaderManager.hpp"
#include <util/Log.hpp>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <core/WindowManager.hpp>

std::string ShaderManager::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    if (stream.fail()) {
        LOG_ERROR("Could not find shader file: " + filepath);
        return "-1";
    }

    std::string line;
    std::stringstream ss;
    while (getline(stream, line)) {
        ss << line << "\n";  
    }
    return ss.str();
}

unsigned int ShaderManager::CompileShader(unsigned int type, std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* sourceStr = source.c_str();
    glShaderSource(id, 1, &sourceStr, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* msg = new char[length];
        glGetShaderInfoLog(id, length, &length, msg);
        
        if (type == GL_VERTEX_SHADER) {
            LOG_ERROR("Failed to compile vertex shader");
        }
        else {
            LOG_ERROR("Failed to compile fragment shader");
        }
        LOG_ERROR(msg);
        glDeleteShader(id);
        delete [] msg;
        return -1;
    }

    return id;
}

unsigned int ShaderManager::CreateShader(std::string& vertex_source, std::string& fragmement_source) {

    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_source);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmement_source);

    if (vs == -1 || fs == -1) {
        glDeleteProgram(program);
        return -1;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void ShaderManager::ClearUniformMap()
{
    for (auto it = m_UniformMap.begin(); it != m_UniformMap.end(); ++it) {
        delete it->second.var;
    }
    m_UniformMap.clear();
}

ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{
    glDeleteProgram(u_ShaderProgramID);
    ClearUniformMap();
}

std::unordered_map<std::string, Uniform> ShaderManager::getUniformMap() const
{
    return m_UniformMap;
}

std::string ShaderManager::getFragmentPath() const
{
    return m_FragmentShaderPath;
}

void ShaderManager::Bind() const
{
    glUseProgram(u_ShaderProgramID);
}

void ShaderManager::Unbind() const
{
    glUseProgram(0);
}

void ShaderManager::SetShader(const std::string& fragment_path)
{

    std::string vertexSource = ParseShader(m_VertexShaderPath);
    std::string fragmentSource = ParseShader(fragment_path);

    if (vertexSource == "-1" || fragmentSource == "-1") {
        return;
    }

    unsigned int newProgram = CreateShader(vertexSource, fragmentSource);

    if (newProgram == -1) {
        return;
    }

    glDeleteProgram(u_ShaderProgramID);
    ClearUniformMap();
    m_FragmentShaderPath = fragment_path;
    u_MouseUniformLoc = -1;
    u_TimeUniformLoc = -1;
    u_ShaderProgramID = newProgram;

    glUseProgram(u_ShaderProgramID);

    GLuint i;
    GLint count;

    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3c or mat4, etc)

    const GLsizei bufSize = 16; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length

    glGetProgramiv(u_ShaderProgramID, GL_ACTIVE_UNIFORMS, &count);

    for (i = 0; i < count; i++)
    {
        glGetActiveUniform(u_ShaderProgramID, (GLuint)i, bufSize, &length, &size, &type, name);
           
        // default uniforms
        if (strcmp(name, "iResolution") == 0) {
            int width, height;
            GLint loc = i;
            glfwGetWindowSize(WindowManager::GetWallpaperWindow(), &width, &height);
            glUniform2i(loc, width, height);
            LOG_INFO("Found default uniform: iResolution");
        }
        else if (strcmp(name, "iTime") == 0) {
            u_TimeUniformLoc = i;
            LOG_INFO("Found default uniform: iTime");
        }
        else if (strcmp(name, "iMouse") == 0) {
            u_MouseUniformLoc = i;
            LOG_INFO("Found default uniform: iMouse");
        }
        else {
            // insert into map non default uniforms for use in ImGUI menu
            switch (type) {
            case GL_FLOAT: {
                m_UniformMap.insert({ name, Uniform{ i, type, (void*)new float(1.0f)}});
                break;
            }
            case GL_INT: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new int(1)} });
                break;
            }
            case GL_INT_VEC2: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new int[2] {0, 0}}});
                break;
            }
            case GL_INT_VEC3: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new int[3]{0, 0, 0}} });
                break;
            }
            case GL_INT_VEC4: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new int[4] {0, 0, 0, 0}}});
                break;
            }
            case GL_FLOAT_VEC2: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new float[2] {0.0f, 0.0f}}});
                break;
            }
            case GL_FLOAT_VEC3: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new float[3] {0.0f, 0.0f, 0.0f}}});
                break;
            }
            case GL_FLOAT_VEC4: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new float[4] {0.0f, 0.0f, 0.0f, 0.0f}}});
                break;
            }
            case GL_BOOL: {
                m_UniformMap.insert({ name, Uniform{i, type, (void*)new bool(false)} });
                break;
            }
            }
        }
    }
    LOG_INFO("Loaded shader " + fragment_path);
}
