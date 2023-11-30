#ifndef APPLICATION_H
#define APPLICATION_H

#include <gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <opengl/ShaderManager.hpp>
#include <opengl/Window.hpp>

class Application
{
private:
    std::unique_ptr<ShaderManager> p_ShaderManager = nullptr;
    std::unique_ptr<Window> p_WallpaperWindow = nullptr;
    std::unique_ptr<Window> p_ImGUIWindow = nullptr;
    wchar_t* p_WallpaperDir;
    void CheckImGUIButtons();
    bool m_isLoadShaderButtonPressed = false;
public:
    Application();
    void Run();
    void SendDefaultUniforms();
};

#endif // !APPLICATION_H

