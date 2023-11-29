#pragma once
#include <GLFW/glfw3.h>
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class WindowManager
{
private:
    static GLFWwindow* m_WallpaperWindow;
    static GLFWwindow* m_ImGUIWindow;

public:
    static void Init();

    inline static GLFWwindow*& GetWallpaperWindow() { return m_WallpaperWindow; };
    inline static GLFWwindow*& GetImGUIWindow() { return m_ImGUIWindow; };
};