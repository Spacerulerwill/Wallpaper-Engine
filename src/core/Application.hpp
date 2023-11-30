#ifndef APPLICATION_H
#define APPLICATION_H

#include <cmath>
#include <gl.h>
#include <GLFW/glfw3.h>
#include <shader/ShaderManager.hpp>
#include <vector>

class Application
{
private:
    static GLFWwindow* p_WallpaperWindow;
    static GLFWwindow* p_ImGUIWindow;
    ShaderManager* shaderManager = nullptr;
    wchar_t* wallpaper_dir;
    void CheckImGUIButtons();
    bool m_isLoadShaderButtonPressed = false;

public:
    Application();

    void Run();
    void ResetWallpaper();
    void SendDefaultUniforms();
};

#endif // !APPLICATION_H

