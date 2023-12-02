#include <fstream>
#include <opengl/ShaderManager.hpp>
#include <sstream>
#include <stdexcept>
#include <util/Log.hpp>

ShaderManager::ShaderManager() {
    bool loadedDefaultVertexShader = LoadShader(GL_VERTEX_SHADER, &uVertexShader, "res/default.vert");

    if (!loadedDefaultVertexShader) {
        throw std::runtime_error("Default vertex shader loading failed. Terminating program.");
    }
}

ShaderManager::~ShaderManager() {
    glDeleteShader(uVertexShader);
    glDeleteProgram(uShaderProgramID);
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

    for (const auto& [key, value] : mUniforms) {
        switch (value.type) {
        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
        case GL_BOOL: {
            delete[] std::any_cast<GLint*>(value.var);
            break;
        }
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4: {
            delete[] std::any_cast<GLfloat*>(value.var);
            break;
        }
        }
    }
    mUniforms.clear();

    GLuint fragmentShader;
    bool loadedFragmentShader = LoadShader(GL_FRAGMENT_SHADER, &fragmentShader, fragmentPath);

    if (!loadedFragmentShader) {
        LOG_ERROR("Failed to load fragment shader: " + fragmentPath);
        glDeleteShader(fragmentShader);
        return false;
    }

    glDeleteProgram(uShaderProgramID);
    uShaderProgramID = glCreateProgram();

    glAttachShader(uShaderProgramID, uVertexShader);
    glAttachShader(uShaderProgramID, fragmentShader);

    glLinkProgram(uShaderProgramID);

    GLint valid = GL_FALSE;
    glValidateProgram(uShaderProgramID);
    glGetProgramiv(uShaderProgramID, GL_VALIDATE_STATUS, &valid);

    if (valid == GL_FALSE) {
        LOG_TRACE("Failed to validate program for fragment shader " + fragmentPath);
        return false;
    }

    glDeleteShader(fragmentShader);
    glUseProgram(uShaderProgramID);

    GLint count;
    GLint size;
    GLenum type;
    const GLsizei bufSize = 16;
    GLchar name[bufSize];
    GLsizei length;
    glGetProgramiv(uShaderProgramID, GL_ACTIVE_UNIFORMS, &count);
    for (GLuint i = 0; i < count; i++)
    {

        glGetActiveUniform(uShaderProgramID, i, bufSize, &length, &size, &type, name);
        if (strcmp(name, "iResolution") == 0 && type == GL_FLOAT_VEC2) {
            glUniform2f(glGetUniformLocation(uShaderProgramID, "iResolution"), static_cast<float>(windowDimensions.width), static_cast<float>(windowDimensions.height));
        }
        else if (strcmp(name, "iTime") == 0 && type == GL_FLOAT) {
            mBuiltinUniformsLocations.time = glGetUniformLocation(uShaderProgramID, "iTime");
        }
        else if (strcmp(name, "iMouse") == 0 && type == GL_FLOAT_VEC2) {
            mBuiltinUniformsLocations.mousePos = glGetUniformLocation(uShaderProgramID, "iMouse");
        }
        else {
            // insert into map non default uniforms for use in ImGUI menu
            switch (type) {
            case GL_FLOAT: {
                GLfloat* val = new GLfloat[1]{};
                glGetUniformfv(uShaderProgramID, static_cast<GLint>(i), val);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_FLOAT, val }));
                break;
            }
            case GL_INT: {
                GLint* val = new GLint[1]{};
                glGetUniformiv(uShaderProgramID, static_cast<GLint>(i), val);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_INT, val }));
                break;
            }
            case GL_INT_VEC2: {
                GLint* vec = new GLint[2]{};
                glGetUniformiv(uShaderProgramID, static_cast<GLint>(i), vec);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_INT_VEC2, vec }));
                break;
            }
            case GL_INT_VEC3: {
                GLint* vec = new GLint[3]{};
                glGetUniformiv(uShaderProgramID, static_cast<GLint>(i), vec);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_INT_VEC3, vec }));
                break;
            }
            case GL_INT_VEC4: {
                GLint* vec = new GLint[4]{};
                glGetUniformiv(uShaderProgramID, static_cast<GLint>(i), vec);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_INT_VEC4, vec }));
                break;
            }
            case GL_FLOAT_VEC2: {
                GLfloat* vec = new GLfloat[2]{};
                glGetUniformfv(uShaderProgramID, static_cast<GLint>(i), vec);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_FLOAT_VEC2, vec }));
                break;
            }
            case GL_FLOAT_VEC3: {
                GLfloat* vec = new GLfloat[3]{};
                glGetUniformfv(uShaderProgramID, static_cast<GLint>(i), vec);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_FLOAT_VEC3, vec }));
                break;
            }
            case GL_FLOAT_VEC4: {
                GLfloat* vec = new GLfloat[4]{};
                glGetUniformfv(uShaderProgramID, static_cast<GLint>(i), vec);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_FLOAT_VEC4, vec }));
                break;
            }
            case GL_BOOL: {
                GLint* val = new GLint[1]{};
                glGetUniformiv(uShaderProgramID, static_cast<GLint>(i), val);
                mUniforms.insert(std::make_pair(std::string(name), Uniform{ static_cast<GLint>(i), GL_BOOL, val }));
                break;
            }
            }
        }
    }

    LOG_INFO("Loaded fragment shader " + fragmentPath);
    return true;
}