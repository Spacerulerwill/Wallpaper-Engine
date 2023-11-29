#pragma once

#include <vector>
#include <cmath>
#include <gl.h>
#include <GLFW/glfw3.h>
#include <shader/ShaderManager.hpp>

class Application
{
private:
	static GLFWwindow* p_WallpaperWindow;
	static GLFWwindow* p_ImGUIWindow;

	ShaderManager* shaderManager = nullptr;

	wchar_t* wallpaper_dir;
	wchar_t* GetWallpaper();
	void SetWallpaper(const wchar_t* path);
	
	void CheckImGUIButtons();
	bool m_isLoadShaderButtonPressed = false;

public:
	Application();

	void Run();
	void ResetWallpaper();
	void SendDefaultUniforms();
};

