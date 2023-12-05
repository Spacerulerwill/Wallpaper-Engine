#include <fstream>
#include <opengl/WallpaperManager.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <util/Log.hpp>
#include <yaml-cpp/yaml.h>

#define DEFAULT_VERTEX_SHADER_PATH "res/vertex.glsl"

WallpaperManager::WallpaperManager() {
    LoadVertexShader();
}

WallpaperManager::~WallpaperManager() {
    glDeleteShader(uVertexShader);
    glDeleteProgram(uShaderProgramID);
}

void WallpaperManager::LoadVertexShader()
{
    std::ifstream stream(DEFAULT_VERTEX_SHADER_PATH);

    if (stream.fail()) {
        throw std::runtime_error("Failed to open file stream to " DEFAULT_VERTEX_SHADER_PATH " to load default vertex shader.");
    }

    std::stringstream ss;
    ss << stream.rdbuf();
    std::string vertexSource = ss.str();

    CompileShader(GL_VERTEX_SHADER, vertexSource, &uVertexShader);
}

bool WallpaperManager::ParseWallpaperSource(const std::string& path, WallpaperSources* out) const
{
    std::ifstream stream(path);

    if (stream.fail()) {
        LOG_ERROR("Failed to open wallpaper: " + path);
        return false;
    }

    std::string line;
    std::stringstream ss[2];
    WallpaperSection type = WallpaperSection::NONE;

    bool foundShaderSection = false;

    while (getline(stream, line)) {
        if (line.find("#section") != std::string::npos) {
            if (line.find("metadata") != std::string::npos) {
                type = WallpaperSection::METADATA;
            }
            else if (line.find("shader") != std::string::npos) {
                foundShaderSection = true;
                type = WallpaperSection::SHADER;
            }
        }
        else if (type != WallpaperSection::NONE) {
            ss[static_cast<int>(type)] << line << "\n";
        }
    }

    if (!foundShaderSection) {
        LOG_ERROR("Wallpaper file must contain #section shader!");
        return false;
    }

    *out = {
        ss[static_cast<int>(WallpaperSection::METADATA)].str(),
        ss[static_cast<int>(WallpaperSection::SHADER)].str()
    };
    return true;
}

bool WallpaperManager::CompileShader(GLenum type, const std::string& source, GLuint* shaderIn) const
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

        std::vector<GLchar> msg(length);
        glGetShaderInfoLog(id, length, &length, msg.data());
        if (type == GL_VERTEX_SHADER) {
            LOG_ERROR("Failed to compile vertex shader");
        }
        else {
            LOG_ERROR("Failed to compile fragment shader");
        }
        LOG_ERROR(msg.data());
        glDeleteShader(id);
        return false;
    }

    *shaderIn = id;
    return true;
}

bool WallpaperManager::ParseWallpaperMetadata(const std::string& metadataYamlSource, WallpaperMetadata& metadata) const
{
    YAML::Node node = YAML::Load(metadataYamlSource);

    try {
        YAML::Node nameNode = node["name"];
        metadata.name = nameNode.as<std::string>();
    }
    catch (const YAML::KeyNotFound& e) {
        metadata.name = "";
    }
    catch (const YAML::BadConversion e) {
        LOG_ERROR("Metadata 'name' must be a string!");
        return false;
    }
}


void WallpaperManager::TrySetWallpaper(const std::string& path, WindowDimensions windowDimensions)
{
    WallpaperSources wallpaperSources{};

    // First of all, parse the wallpaper soure file into its seperate components (fragment shader and config yaml)
    bool parsed = ParseWallpaperSource(path, &wallpaperSources);
    if (!parsed) {
        return;
    }

    // Try and compile the fragment shader
    GLuint fragmentShader = 0;
    bool compiled = CompileShader(GL_FRAGMENT_SHADER, wallpaperSources.fragmentShaderSource, &fragmentShader);
    if (!compiled) {
        return;
    }

    // Try and parse the metadata yaml
    WallpaperMetadata metadata{};

    if (wallpaperSources.metadataYamlSource != "") {
        bool metadataParsed = false;

        try {
            metadataParsed = ParseWallpaperMetadata(wallpaperSources.metadataYamlSource, metadata);
        }
        catch (YAML::Exception e) {
            glDeleteShader(fragmentShader);
            LOG_ERROR(e.what());
            return;
        }

        if (!metadataParsed) {
            glDeleteShader(fragmentShader);
            return;
        }
    }

    // Try and create the new prgoram
    GLuint program = glCreateProgram();

    glAttachShader(program, uVertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint valid = GL_FALSE;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &valid);

    if (valid == GL_FALSE) {
        LOG_TRACE("Failed to validate shader program for wallpaper " + path);
        glDeleteProgram(program);
        glDeleteShader(fragmentShader);
        return;
    }

    // We have made it without any errors so we are safe to remove previous shader
    glDeleteProgram(uShaderProgramID);
    glDeleteShader(fragmentShader);
    uShaderProgramID = program;
    glUseProgram(uShaderProgramID);
    mMetadata = metadata;

    // Clear any uniforms from the previous shader
    mIntUniforms.clear();
    mFloatUniforms.clear();
    mBoolUniforms.clear();

    // Gather our shaders uniform values and store them in the mUniforms map
    mBuiltinUniformsLocations = BuiltinUniformsLocations{};
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
            case GL_INT: {
                GLint val{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformiv(uShaderProgramID, loc, &val);
                mIntUniforms.insert(std::make_pair(std::string(name), Uniform<GLint>(1, &val, loc)));
                break;
            }
            case GL_INT_VEC2: {
                GLint val[2]{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformiv(uShaderProgramID, loc, &val[0]);
                mIntUniforms.insert(std::make_pair(std::string(name), Uniform<GLint>(2, &val[0], loc)));
                break;
            }
            case GL_INT_VEC3: {
                GLint val[3]{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformiv(uShaderProgramID, loc, &val[0]);
                mIntUniforms.insert(std::make_pair(std::string(name), Uniform<GLint>(3, &val[0], loc)));
                break;
            }
            case GL_INT_VEC4: {
                GLint val[4]{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformiv(uShaderProgramID, loc, &val[0]);
                mIntUniforms.insert(std::make_pair(std::string(name), Uniform<GLint>(4, &val[0], loc)));
                break;
            }
            case GL_FLOAT: {
                GLfloat val{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformfv(uShaderProgramID, loc, &val);
                mFloatUniforms.insert(std::make_pair(std::string(name), Uniform<GLfloat>(1, &val, loc)));
                break;
            }
            case GL_FLOAT_VEC2: {
                GLfloat val[2]{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformfv(uShaderProgramID, loc, &val[0]);
                mFloatUniforms.insert(std::make_pair(std::string(name), Uniform<GLfloat>(2, &val[0], loc)));
                break;
            }
            case GL_FLOAT_VEC3: {
                GLfloat val[3]{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformfv(uShaderProgramID, loc, &val[0]);
                mFloatUniforms.insert(std::make_pair(std::string(name), Uniform<GLfloat>(3, &val[0], loc)));
                break;
            }
            case GL_FLOAT_VEC4: {
                GLfloat val[4]{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformfv(uShaderProgramID, loc, &val[0]);
                mFloatUniforms.insert(std::make_pair(std::string(name), Uniform<GLfloat>(4, &val[0], loc)));
                break;
            }
            case GL_BOOL: {
                GLint val{};
                GLint loc = glGetUniformLocation(uShaderProgramID, name);
                glGetUniformiv(uShaderProgramID, loc, &val);
                mBoolUniforms.insert(std::make_pair(std::string(name), Uniform<GLboolean>(1, reinterpret_cast<GLboolean*>(&val), loc)));
                break;
            }
            }
        }
    }
    hasWallpaper = true;
    LOG_INFO("Loaded wallpaper: " + path);
}
