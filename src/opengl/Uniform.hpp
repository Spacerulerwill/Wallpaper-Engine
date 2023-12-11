#ifndef UNIFORM_H
#define UNIFORM_H

#include <gl.h>
#include <string>

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
    std::string name{};
    T min{};
    T max{};
};

template<typename T>
struct Uniform {
    GLint location = -1;
    std::vector<T> elements{};
    UniformMetadata<T> metadata{};
    Uniform(size_t size, std::vector<T> values, GLint location, const UniformMetadata<T>& metadata) : elements(std::move(values)), location(location), metadata(metadata) {}
    Uniform() {}
};

#endif // !UNIFORM_H
