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

bool WallpaperManager::ParseWallpaperMetadata(
    const std::string& metadataYamlSource,
    WallpaperMetadata& wallpaperMetadata,
    std::unordered_map <std::string, Uniform<GLint>>& intUniforms,
    std::unordered_map <std::string, Uniform<GLfloat>>& floatUniforms,
    std::unordered_map <std::string, Uniform<GLboolean>>& boolUniforms
) const {
    YAML::Node node = YAML::Load(metadataYamlSource);

    try {
        YAML::Node nameNode = node["name"];
        wallpaperMetadata.name = nameNode.as<std::string>();
    }
    catch (const YAML::KeyNotFound&) {
        wallpaperMetadata.name = "";
    }
    catch (const YAML::BadConversion e) {
        LOG_ERROR("Metadata 'name' must be a string!");
        return false;
    }

    // Process float uniforms
    YAML::Node uniforms = node["uniforms"];
    auto floatUniformMetadata = uniforms["float"].as<std::unordered_map<std::string, UniformMetadata<GLfloat>>>();
    for (auto it = floatUniformMetadata.begin(); it != floatUniformMetadata.end(); ++it) {
        Uniform<GLfloat> uniform{};
        uniform.metadata = it->second;
        floatUniforms.insert(std::make_pair(it->first, uniform));
    }

    // Process int uniforms
    auto intUniformMetadata = uniforms["int"].as<std::unordered_map<std::string, UniformMetadata<GLint>>>();
    for (auto it = intUniformMetadata.begin(); it != intUniformMetadata.end(); ++it) {
        Uniform<GLint> uniform{};
        uniform.metadata = it->second;
        intUniforms.insert(std::make_pair(it->first, uniform));
    }

    // Process bool uniforms
    auto boolUniformMetadata = uniforms["bool"].as<std::unordered_map<std::string, UniformMetadata<GLboolean>>>();
    for (auto it = boolUniformMetadata.begin(); it != boolUniformMetadata.end(); ++it) {
        Uniform<GLboolean> uniform{};
        uniform.metadata = it->second;
        boolUniforms.insert(std::make_pair(it->first, uniform));
    }
    return true;
}

void WallpaperManager::AddIntUniform(std::string name, size_t count)
{
    std::vector<GLint> val(count);
    GLint loc = glGetUniformLocation(uShaderProgramID, name.c_str());
    glGetUniformiv(uShaderProgramID, loc, val.data());
    auto find = mIntUniforms.find(name);
    if (find == mIntUniforms.end()) {
        UniformMetadata<GLint> metadata{
           .name = std::string(name),
           .min = DEFAULT_INT_SLIDER_MIN,
           .max = DEFAULT_INT_SLIDER_MAX,
        };
        Uniform<GLint> uniform(1, val, loc, metadata);
        mIntUniforms.insert(std::make_pair(name, uniform));
    }
    else {
        (*find).second.location = loc;
        (*find).second.elements = std::move(val);
    }
}


void WallpaperManager::AddFloatUniform(std::string name, size_t count)
{
    std::vector<GLfloat> val(count);
    GLint loc = glGetUniformLocation(uShaderProgramID, name.c_str());
    glGetUniformfv(uShaderProgramID, loc, val.data());
    auto find = mFloatUniforms.find(name);
    if (find == mFloatUniforms.end()) {
        UniformMetadata<GLfloat> metadata{
            .name = std::string(name),
            .min = DEFAULT_FLOAT_SLIDER_MIN,
            .max = DEFAULT_FLOAT_SLIDER_MAX,
        };
        Uniform<GLfloat> uniform(1, val, loc, metadata);
        mFloatUniforms.insert(std::make_pair(name, uniform));
    }
    else {
        (*find).second.location = loc;
        (*find).second.elements = std::move(val);
    }
}

void WallpaperManager::AddBoolUniform(std::string name, size_t count)
{
    std::vector<GLboolean> val(count);
    GLint loc = glGetUniformLocation(uShaderProgramID, name.c_str());
    glGetUniformiv(uShaderProgramID, loc, reinterpret_cast<GLint*>(val.data()));
    auto find = mBoolUniforms.find(name);
    if (find == mBoolUniforms.end()) {
        UniformMetadata<GLboolean> metadata{
            .name = std::string(name),
        };
        Uniform<GLboolean> uniform(1, val, loc, metadata);
        mBoolUniforms.insert(std::make_pair(name, uniform));
    }
    else {
        (*find).second.location = loc;
        (*find).second.elements = std::move(val);
    }
}

bool WallpaperManager::TrySetWallpaper(const std::string& path, WindowDimensions windowDimensions)
{
    WallpaperSources wallpaperSources{};

    // First of all, parse the wallpaper soure file into its seperate components (fragment shader and config yaml)
    bool parsed = ParseWallpaperSource(path, &wallpaperSources);
    if (!parsed) {
        return false;
    }

    // Try and compile the fragment shader
    GLuint fragmentShader = 0;
    bool compiled = CompileShader(GL_FRAGMENT_SHADER, wallpaperSources.fragmentShaderSource, &fragmentShader);
    if (!compiled) {
        return false;
    }

    // Try and parse the metadata yaml
    std::unordered_map <std::string, Uniform<GLint>> intUniforms;
    std::unordered_map <std::string, Uniform<GLfloat>> floatUniforms;
    std::unordered_map <std::string, Uniform<GLboolean>> boolUniforms;
    WallpaperMetadata metadata{};

    if (wallpaperSources.metadataYamlSource != "") {
        bool metadataParsed = false;

        try {
            metadataParsed = ParseWallpaperMetadata(wallpaperSources.metadataYamlSource, metadata, intUniforms, floatUniforms, boolUniforms);
        }
        catch (YAML::Exception e) {
            glDeleteShader(fragmentShader);
            LOG_ERROR(e.what());
            return false;
        }

        if (!metadataParsed) {
            glDeleteShader(fragmentShader);
            return false;
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
        return false;
    }

    // We have made it without any errors so we are safe to remove previous shader
    glDeleteShader(fragmentShader);
    UnloadCurrentWallpaper();
    uShaderProgramID = program;
    glUseProgram(uShaderProgramID);
    mMetadata = metadata;
    mIntUniforms = std::move(intUniforms);
    mFloatUniforms = std::move(floatUniforms);
    mBoolUniforms = std::move(boolUniforms);

    // Gather our shaders uniform values and store them in the mUniforms map
    mBuiltinUniformsLocations = BuiltinUniformsLocations{};
    GLint count;
    GLint size;
    GLenum type;
    const GLsizei bufSize = 16;
    GLchar name[bufSize];
    GLsizei length;
    glGetProgramiv(uShaderProgramID, GL_ACTIVE_UNIFORMS, &count);
    for (GLint i = 0; i < count; i++)
    {
        glGetActiveUniform(uShaderProgramID, static_cast<GLuint>(i), bufSize, &length, &size, &type, name);
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
                AddIntUniform(name, 1);
                break;
            }
            case GL_INT_VEC2: {
                AddIntUniform(name, 2);
                break;
            }
            case GL_INT_VEC3: {
                AddIntUniform(name, 3);
                break;
            }
            case GL_INT_VEC4: {
                AddIntUniform(name, 4);
                break;
            }
            case GL_FLOAT: {
                AddFloatUniform(name, 1);
                break;
            }
            case GL_FLOAT_VEC2: {
                AddFloatUniform(name, 2);
                break;
            }
            case GL_FLOAT_VEC3: {
                AddFloatUniform(name, 3);
                break;
            }
            case GL_FLOAT_VEC4: {
                AddFloatUniform(name, 4);
                break;
            }
            case GL_BOOL: {
                AddBoolUniform(name, 1);
                break;
            }
            }
        }
    }
    hasWallpaper = true;
    LOG_INFO("Loaded wallpaper: " + path);
    return true;
}

void WallpaperManager::UnloadCurrentWallpaper()
{
    glDeleteProgram(uShaderProgramID);
    mMetadata = WallpaperMetadata{};
    mIntUniforms.clear();
    mFloatUniforms.clear();
    mBoolUniforms.clear();
}
