cmake_minimum_required(VERSION 3.28.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(WallpaperEngine VERSION 1.0)

add_executable(${PROJECT_NAME} 
    lib/glad/gl.c
    lib/imgui/imgui_impl_glfw.cpp
    lib/imgui/imgui_impl_opengl3.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    src/main.cpp
    src/core/Application.cpp
    src/core/Application.hpp
    src/core/WindowManager.cpp
    src/core/WindowManager.hpp
    src/shader/ShaderManager.cpp
    src/shader/ShaderManager.hpp
    src/util/Constants.hpp
    src/util/Log.cpp
    src/util/Log.hpp
    src/vertex/IndexBuffer.cpp
    src/vertex/IndexBuffer.hpp
    src/vertex/VertexArray.cpp
    src/vertex/VertexArray.hpp
    src/vertex/VertexBuffer.cpp
    src/vertex/VertexBuffer.hpp
    src/vertex/VertexBufferLayout.cpp
    src/vertex/VertexBufferLayout.hpp
)

add_subdirectory(submodules/glfw)
add_subdirectory(submodules/spdlog)

target_include_directories(${PROJECT_NAME} 
    PUBLIC submodules/glfw/include
    PUBLIC submodules/spdlog/include
    PUBLIC include/imgui
    PUBLIC include/glad
    PUBLIC include/imgui
    PUBLIC include
    PUBLIC src
)

target_link_directories(${PROJECT_NAME}
    PRIVATE submodules/glfw/src
    PRIVATE submodules/spdlog/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC spdlog
    PUBLIC glfw
)

file(COPY res DESTINATION ${CMAKE_BINARY_DIR})