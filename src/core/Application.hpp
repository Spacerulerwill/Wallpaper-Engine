#ifndef APPLICATION_H
#define APPLICATION_H

#include <gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <opengl/WallpaperManager.hpp>
#include <opengl/Window.hpp>

class Application
{
private:
    std::unique_ptr<WallpaperManager> pWallpaperManager = nullptr;
    std::unique_ptr<Window> pWallpaperWindow = nullptr;
    std::unique_ptr<Window> pImGUIWindow = nullptr;
    std::wstring mOriginalWallpaperPath;
    void ProcessImGUI() const;
    void DrawImGUIControlMenu();
    void UpdateUniforms() const;
    bool mIsLoadWallpaperButtonPressed = false;
    bool mIsUnloadWallpaperButtonPressed = false;
    GLuint mVAO{};
public:
    Application();
    void Run();
    void Cleanup() const;
};

#endif // !APPLICATION_H

