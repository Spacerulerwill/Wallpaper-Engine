#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <util/Log.hpp>
#include <gl.h>
#include <unordered_map>
#include <opengl/Window.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <concepts>
#include <yaml-cpp/yaml.h>

constexpr float DEFAULT_FLOAT_SLIDER_MIN = 0.0f;
constexpr float DEFAULT_FLOAT_SLIDER_MAX = 100.0f;

constexpr int DEFAULT_INT_SLIDER_MIN = 0;
constexpr int DEFAULT_INT_SLIDER_MAX = 100;

template<typename T>
struct UniformMetadata;

template<>
struct UniformMetadata<GLboolean> {
    std::string name;
};

template<typename T>
struct UniformMetadata {
    std::string name;
    T min;
    T max;
};

template<typename T>
struct Uniform {
    GLint location;
    std::vector<T> elements;
    UniformMetadata<T> metadata{};
    Uniform(size_t size, std::vector<T> values, GLint location, const UniformMetadata<T>& metadata) : elements(std::move(values)), location(location), metadata(metadata) {}
    Uniform() : location(-1), elements(std::vector<T>()), metadata(UniformMetadata<T>{}) {}
};

struct BuiltinUniformsLocations {
    GLint time = static_cast<GLint>(GL_INVALID_INDEX);
    GLint mousePos = static_cast<GLint>(GL_INVALID_INDEX);
};

enum WallpaperSection {
    NONE = -1,
    METADATA = 0,
    SHADER = 1
};

struct WallpaperSources {
    std::string metadataYamlSource;
    std::string fragmentShaderSource;
};

struct WallpaperMetadata {
    std::string name;
};

class WallpaperManager
{
private:
    GLuint uShaderProgramID = 0;
    GLuint uVertexShader = 0;

    void LoadVertexShader();
    bool ParseWallpaperSource(const std::string& filepath, WallpaperSources* out) const;
    bool CompileShader(GLenum type, const std::string& source, GLuint* shaderIn) const;
    bool ParseWallpaperMetadata(
        const std::string& metadataYamlSource,
        WallpaperMetadata& wallpaperMetadata,
        std::unordered_map <std::string, Uniform<GLint>>& intUniforms,
        std::unordered_map <std::string, Uniform<GLfloat>>& floatUniforms,
        std::unordered_map <std::string, Uniform<GLboolean>>& boolUniforms
    ) const;

    void AddIntUniform(std::string name, size_t count);
    void AddFloatUniform(std::string name, size_t count);
    void AddBoolUniform(std::string name, size_t count);

public:
    WallpaperManager();
    ~WallpaperManager();
    void TrySetWallpaper(const std::string& path, WindowDimensions);

    std::unordered_map<std::string, Uniform<GLint>> mIntUniforms;
    std::unordered_map<std::string, Uniform<GLboolean>> mBoolUniforms;
    std::unordered_map<std::string, Uniform<GLfloat>> mFloatUniforms;

    WallpaperMetadata mMetadata{};
    BuiltinUniformsLocations mBuiltinUniformsLocations;
    bool hasWallpaper = false;

    WallpaperManager(const WallpaperManager& arg) = delete;
    WallpaperManager(const WallpaperManager&& arg) = delete;
    WallpaperManager& operator=(const WallpaperManager& arg) = delete;
    WallpaperManager& operator=(const WallpaperManager&& arg) = delete;
};

namespace YAML {
    template<>
    struct convert<UniformMetadata<GLboolean>> {
        static Node encode(const UniformMetadata<GLboolean>& rhs) {
            Node node;
            node["name"] = rhs.name;
            return node;
        }

        static bool decode(const Node& node, UniformMetadata<GLboolean>& rhs) {
            if (!node.IsMap()) {
                return false;
            }

            rhs.name = node["name"].as<std::string>();
            return true;
        }
    };


    template<typename T>
    struct convert<UniformMetadata<T>> {
        static Node encode(const UniformMetadata<T>& rhs) {
            Node node;
            node["name"] = rhs.name;
            node["min"] = rhs.min;
            node["max"] = rhs.max;
            return node;
        }

        static bool decode(const Node& node, UniformMetadata<T>& rhs) {
            if (!node.IsMap()) {
                return false;
            }

            rhs.name = node["name"].as<std::string>();
            rhs.min = node["min"].as<T>();
            rhs.max = node["max"].as<T>();

            return true;
        }
    };
}

#endif // !SHADER_MANAGER_H
