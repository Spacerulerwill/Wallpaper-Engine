#include <fstream>
#include <opengl/ShaderManager.hpp>
#include <sstream>
#include <system_error>
#include <util/Log.hpp>

ShaderManager::ShaderManager() {
    bool loadedDefaultVertexShader = LoadShader(GL_VERTEX_SHADER, &u_VertexShader, "res/default.vert");

    if (!loadedDefaultVertexShader) {
        throw std::runtime_error("Default vertex shader loading failed. Terminating program.");
    }
}

ShaderManager::~ShaderManager() {
    glDeleteShader(u_VertexShader);
    glDeleteProgram(u_ShaderProgramID);
}

bool ShaderManager::LoadShader(GLenum shaderType, GLuint* shader, const std::string& path) {
    std::string source;
    bool parsed = ParseShader(path, &source);
    if (!parsed) {
        return false;
    }

    bool compiled = CompileShader(shaderType, source, shader);
    if (!compiled) {
        return false;
    }

    return true;
}

bool ShaderManager::ParseShader(const std::string& fragmentPath, std::string* out)
{
    std::ifstream stream(fragmentPath);

    if (stream.fail()) {
        LOG_ERROR("Could not find shader file: " + fragmentPath);
        return false;
    }

    std::string line;
    std::stringstream ss;

    while (getline(stream, line)) {
        ss << line << "\n";
    }

    *out = ss.str();


    if (stream.bad()) {
        LOG_ERROR("Failed to parse shader " + fragmentPath + " due to bad stream!");
        return false;
    }
    return true;
}

bool ShaderManager::CompileShader(GLenum type, const std::string& source, GLuint* shaderIn)
{
    GLuint id = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(id, 1, &sourceCStr, nullptr);
    glCompileShader(id);

    GLint result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        GLint length;
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
        delete[] msg;

        return false;
    }

    *shaderIn = id;
    return true;
}

bool ShaderManager::SetFragmentShader(const std::string& fragmentPath, WindowDimensions windowDimensions)
{
    GLuint fragmentShader;
    bool loadedFragmentShader = LoadShader(GL_FRAGMENT_SHADER, &fragmentShader, fragmentPath);

    if (!loadedFragmentShader) {
        LOG_ERROR("Failed to load fragment shader: " + fragmentPath);
        glDeleteShader(fragmentShader);
        return false;
    }

    glDeleteProgram(u_ShaderProgramID);
    u_ShaderProgramID = glCreateProgram();

    glAttachShader(u_ShaderProgramID, u_VertexShader);
    glAttachShader(u_ShaderProgramID, fragmentShader);

    glLinkProgram(u_ShaderProgramID);

    GLint valid = GL_FALSE;
    glValidateProgram(u_ShaderProgramID);
    glGetProgramiv(u_ShaderProgramID, GL_VALIDATE_STATUS, &valid);

    if (valid == GL_FALSE) {
        LOG_TRACE("Failed to validate program for fragment shader " + fragmentPath);
        return false;
    }

    glDeleteShader(fragmentShader);
    glUseProgram(u_ShaderProgramID);

    BuiltinUniformsLocations.time = glGetUniformLocation(u_ShaderProgramID, "iTime");
    BuiltinUniformsLocations.resolution = glGetUniformLocation(u_ShaderProgramID, "iResolution");

    if (BuiltinUniformsLocations.resolution != -1) {
        glUniform2f(BuiltinUniformsLocations.resolution, static_cast<float>(windowDimensions.width), static_cast<float>(windowDimensions.height));
    }
    BuiltinUniformsLocations.mousePos = glGetUniformLocation(u_ShaderProgramID, "iMouse");

    LOG_INFO("Loaded fragment shader " + fragmentPath);
    return true;
}