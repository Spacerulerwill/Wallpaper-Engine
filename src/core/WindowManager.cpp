#include "WindowManager.hpp"
#include <util/Log.hpp>

GLFWwindow* WindowManager::m_WallpaperWindow = nullptr;
GLFWwindow* WindowManager::m_ImGUIWindow = nullptr;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    HWND p = FindWindowEx(hwnd, NULL, TEXT("SHELLDLL_DefView"), NULL);
    HWND* ret = (HWND*)lParam;

    if (p)
    {
        // Gets the WorkerW Window after the current one.
        *ret = FindWindowEx(NULL, hwnd, TEXT("WorkerW"), NULL);
    }
    return true;
}

HWND get_wallpaper_hwnd()
{
    // Fetch the Progman window
    HWND progman = FindWindow(TEXT("ProgMan"), NULL);
    // Send 0x052C to Progman. This message directs Progman to spawn a 
    // WorkerW behind the desktop icons. If it is already there, nothing 
    // happens.
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    // We enumerate all Windows, until we find one, that has the SHELLDLL_DefView 
    // as a child. 
    // If we found that window, we take its next sibling and assign it to workerw.
    HWND wallpaper_hwnd = nullptr;
    EnumWindows(EnumWindowsProc, (LPARAM)&wallpaper_hwnd);
    // Return the handle you're looking for.
    return wallpaper_hwnd;
}

RECT get_desktop_rect() {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    return desktop;
}

void WindowManager::Init()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    RECT desktop = get_desktop_rect();

	GLFWwindow* wallpaper_window = glfwCreateWindow(desktop.right, desktop.bottom, "Wallpaper Engine", NULL, NULL);

	if (wallpaper_window == NULL)
	{
        LOG_CRITICAL("Failed to create wallpaper window! Aborting...");
		glfwTerminate();
		std::exit(-1);
	}
    m_WallpaperWindow = wallpaper_window;
    glfwMakeContextCurrent(m_WallpaperWindow);

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* imgui_window = glfwCreateWindow(400, 400, "Wallpaper Engine", NULL, NULL);

    if (imgui_window == NULL)
    {
        LOG_CRITICAL("Failed to create ImGUI window! Aborting...");
        glfwTerminate();
        std::exit(-1);
    }
    m_ImGUIWindow = imgui_window;

    HWND windows_wallpaper_hwnd = get_wallpaper_hwnd();
    HWND wallpaper_glfw_hwnd = glfwGetWin32Window(m_WallpaperWindow);

    SetParent(wallpaper_glfw_hwnd, windows_wallpaper_hwnd);
}