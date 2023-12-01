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
    std::unique_ptr<ShaderManager> pShaderManager = nullptr;
    std::unique_ptr<Window> pWallpaperWindow = nullptr;
    std::unique_ptr<Window> pImGUIWindow = nullptr;
    wchar_t* pWallpaperDir;
    void ProcessImGUI();
    void UpdateBuiltinUniforms();
    bool mIsLoadShaderButtonPressed = false;
public:
    Application();
    void Run();
};

#endif // !APPLICATION_H

