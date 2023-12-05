#ifndef APPLICATION_H
#define APPLICATION_H

#include <gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <opengl/WallpaperManager.hpp>
#include <opengl/Window.hpp>

class Application
{
private:
    std::unique_ptr<WallpaperManager> pWallpaperManager = nullptr;
    std::unique_ptr<Window> pWallpaperWindow = nullptr;
    std::unique_ptr<Window> pImGUIWindow = nullptr;
    wchar_t* pWallpaperDir;
    void ProcessImGUI();
    void DrawImGUIControlMenu();
    void UpdateUniforms();
    bool mIsLoadShaderButtonPressed = false;
    bool mIsUnloadShaderButtonPressed = false;
public:
    Application();
    void Run();
};

#endif // !APPLICATION_H

