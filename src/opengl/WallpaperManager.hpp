#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <gl.h>
#include <unordered_map>
#include <opengl/Window.hpp>
#include <string>
#include <unordered_map>
#include <vector>

template<typename T>
struct Uniform {
    GLint location;
    std::vector<T> elements;

    Uniform(size_t size, T* value, GLint location) : elements(std::vector<T>(value, value + size)), location(location) {

    }
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
    bool ParseWallpaperMetadata(const std::string& metadataYamlSource, WallpaperMetadata& metadata) const;
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

#endif // !SHADER_MANAGER_H