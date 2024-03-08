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
    src/opengl/WallpaperManager.cpp
    src/opengl/WallpaperManager.hpp
    src/opengl/Uniform.hpp
    src/opengl/Window.cpp
    src/opengl/Window.hpp
    src/opengl/Texture.cpp
    src/opengl/Texture.hpp
    src/util/Log.cpp
    src/util/Log.hpp
    src/util/OS.cpp
    src/util/OS.hpp
)

add_subdirectory(lib/submodules/glfw)
add_subdirectory(lib/submodules/spdlog)
add_subdirectory(lib/submodules/yaml-cpp)

target_include_directories(${PROJECT_NAME} 
    SYSTEM PRIVATE lib/submodules/glfw/include
    SYSTEM PRIVATE lib/submodules/spdlog/include
    SYSTEM PRIVATE lib/submodules/yaml-cpp/include
    SYSTEM PRIVATE include/imgui
    SYSTEM PRIVATE include/glad
    SYSTEM PRIVATE include
    SYSTEM PRIVATE src
)

target_link_directories(${PROJECT_NAME}
    PRIVATE lib/submodules/glfw/src
    PRIVATE lib/submodules/spdlog/src
    PRIVATE lib/submodules/yaml-cpp/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC spdlog
    PUBLIC glfw
    PUBLIC yaml-cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE /W4 /external:W0 /wd4996)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>)
if(WIN32)
    # copy dynamic libraries to executable location
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/lib/windows/dynamic $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    # link static libraries
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/windows/irrKlang.lib)
    set_property(TARGET ${PROJECT_NAME}  PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(WIN32)
if(WIN32)
       set_property(TARGET ${PROJECT_NAME}  PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(WIN32)