#include "Application.hpp"
#include <vector>
#include <unordered_map>
#include <Windows.h>
#include <ShObjIdl.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vertex/IndexBuffer.hpp>
#include <vertex/VertexArray.hpp>
#include <vertex/VertexBuffer.hpp>
#include <vertex/VertexBufferLayout.hpp>

#include <core/WindowManager.hpp>
#include <shader/ShaderManager.hpp>
#include <util/Log.hpp>
#include <util/Constants.hpp>

// static variable definitions
std::unique_ptr<Application> Application::s_Instance = nullptr;
GLFWwindow* Application::p_WallpaperWindow = nullptr;
GLFWwindow* Application::p_ImGUIWindow = nullptr;



Application::Application()
{
	wallpaper_dir = GetWallpaper();
}

wchar_t* Application::GetWallpaper()
{
	wchar_t buf[512];
	SystemParametersInfoW(SPI_GETDESKWALLPAPER, 512, buf, 0);
	return &buf[0];
}

void Application::ResetWallpaper()
{
	SetWallpaper(wallpaper_dir);
}

void Application::SetWallpaper(const wchar_t* path)
{
	SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)path, SPIF_UPDATEINIFILE);
}

bool openFile(std::string* sFilePath)
{
	//  CREATE FILE OBJECT INSTANCE
	HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(f_SysHr))
		return FALSE;

	// CREATE FileOpenDialog OBJECT
	IFileOpenDialog* f_FileSystem;
	f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
	if (FAILED(f_SysHr)) {
		CoUninitialize();
		return FALSE;
	}

	//  SHOW OPEN FILE DIALOG WINDOW
	f_SysHr = f_FileSystem->Show(NULL);
	if (FAILED(f_SysHr)) {
		f_FileSystem->Release();
		CoUninitialize();
		return FALSE;
	}

	//  RETRIEVE FILE NAME FROM THE SELECTED ITEM
	IShellItem* f_Files;
	f_SysHr = f_FileSystem->GetResult(&f_Files);
	if (FAILED(f_SysHr)) {
		f_FileSystem->Release();
		CoUninitialize();
		return FALSE;
	}

	//  STORE AND CONVERT THE FILE NAME
	PWSTR f_Path;
	f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
	if (FAILED(f_SysHr)) {
		f_Files->Release();
		f_FileSystem->Release();
		CoUninitialize();
		return FALSE;
	}

	//  FORMAT AND STORE THE FILE PATH
	std::wstring path(f_Path);
	std::string c(path.begin(), path.end());
	*sFilePath = c;

	//  SUCCESS, CLEAN UP
	CoTaskMemFree(f_Path);
	f_Files->Release();
	f_FileSystem->Release();
	CoUninitialize();
	return TRUE;
}

void Application::Run()
{
	Log::Init();

	if (!glfwInit()) {
		LOG_CRITICAL("Failed to intialise GLFW! Aborting...");
		std::exit(-1);
	}

	// initialise window
	// ----------------
	WindowManager::Init();
	p_WallpaperWindow = WindowManager::GetWallpaperWindow();
	p_ImGUIWindow = WindowManager::GetImGUIWindow();

	// initialise GLAD
	// ---------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG_CRITICAL("Failed to initialise GLAD! Aborting...");
		std::exit(-1);
	}

	// create buffers for rendering the quad
	VertexBufferLayout layout;
	layout.AddAttribute<float>(2);
	layout.AddAttribute<float>(2);

	VertexBuffer VBO(vertices, sizeof(vertices));
	IndexBuffer EBO(indices, sizeof(indices));

	VertexArray VAO;
	VAO.AddBuffer(VBO, layout);

	VAO.Bind();
	EBO.Bind();
	
	shaderManager = new ShaderManager();
	shaderManager->SetShader("default.frag");

	// ImGui context creationg and start up
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL; // disable imgui.ini
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(p_ImGUIWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	
	// application loop
	// ---------------
	while (!glfwWindowShouldClose(p_ImGUIWindow)) {		
		// render wallpaper window
		// -----------------------
		glfwMakeContextCurrent(p_WallpaperWindow);
		SendDefaultUniforms();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// update non default uniforms
		std::unordered_map<std::string, Uniform>map = shaderManager->getUniformMap();
		for (auto it = map.begin(); it != map.end(); ++it) {
			unsigned int loc = it->second.location;
			GLenum type = it->second.type;

			switch (type) {
			case GL_FLOAT: {
				glUniform1f(loc, *(float*)it->second.var);
				break;
			}
			case GL_INT: {
				glUniform1i(loc, *(int*)it->second.var);
				break;
			}
			case GL_INT_VEC2: {
				glUniform2iv(loc, 1, (int*)it->second.var);
				break;
			}
			case GL_INT_VEC3: {
				glUniform3iv(loc, 1, (int*)it->second.var);
				break;
			}
			case GL_INT_VEC4: {
				glUniform4iv(loc, 1, (int*)it->second.var);
				break;
			}
			case GL_FLOAT_VEC2: {
				glUniform2fv(loc, 1, (float*)it->second.var);
				break;
			}
			case GL_FLOAT_VEC3: {
				glUniform3fv(loc, 1, (float*)it->second.var);
				break;
			}
			case GL_FLOAT_VEC4: {
				glUniform4fv(loc, 1, (float*)it->second.var);
				break;
			}
			case GL_BOOL: {
				glUniform1i(loc, *(bool*)it->second.var);
				break;
			}
			}
		}

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(p_WallpaperWindow);

		CheckImGUIButtons();

		// render ImGUI window
		// -----------------------
		glfwMakeContextCurrent(p_ImGUIWindow);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("Control Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		m_isLoadShaderButtonPressed = ImGui::Button("Load Shader");
		ImGui::SameLine();
		ImGui::Text(shaderManager->getFragmentPath().c_str());
			
		// create ImGUI elements for non default uniforms
		for (auto it = map.begin(); it != map.end(); ++it) {
			const char* name = it->first.c_str();
			unsigned int loc = it->second.location;
			GLenum type = it->second.type;
			switch (type) {
			case GL_FLOAT: {
				ImGui::SliderFloat(name, (float*)it->second.var, 0.0f, 10.0f);
				break;
			}
			case GL_INT: {
				ImGui::SliderInt(name, (int*)it->second.var, 0, 100);
				break;
			}
			case GL_INT_VEC2: {
				ImGui::SliderInt2(name, (int*)it->second.var, 0, 100);
				break;
			}
			case GL_INT_VEC3: {
				ImGui::SliderInt3(name, (int*)it->second.var, 0, 100);
				break;
			}
			case GL_INT_VEC4: {
				ImGui::SliderInt4(name, (int*)it->second.var, 0, 100);
				break;
			}
			case GL_FLOAT_VEC2: {
				ImGui::SliderFloat2(name, (float*)it->second.var, 0, 10.0f);
				break;
			}
			case GL_FLOAT_VEC3: {
				ImGui::SliderFloat3(name, (float*)it->second.var, 0, 10.0f);
				break;
			}
			case GL_FLOAT_VEC4: {
				ImGui::SliderFloat4(name, (float*)it->second.var, 0, 10.0f);
				break;
			}
			case GL_BOOL: {
				ImGui::Checkbox(name, (bool*)it->second.var);
				break;
			}
			}
		}

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(p_ImGUIWindow);

		glfwPollEvents();

	}
	glfwTerminate();
	SetWallpaper(wallpaper_dir);
	delete shaderManager;
}


void Application::CheckImGUIButtons()
{
	if (m_isLoadShaderButtonPressed) {

		std::string newPath = std::string();
		bool success = openFile(&newPath);

		if (success) {
			shaderManager->SetShader(newPath);
		}
		else {
			LOG_ERROR("Failed to load shader");
		}
	}
}

void Application::SendDefaultUniforms()
{
	// send mouse uniform if needed
	if (shaderManager->u_MouseUniformLoc != -1) {
		POINT p;
		if (GetCursorPos(&p))
		{
			glUniform2f(shaderManager->u_MouseUniformLoc, static_cast<GLfloat>(p.x), static_cast<GLfloat>(p.y));
		}
	}
		
	// send time if needed
	if (shaderManager->u_TimeUniformLoc) {
		glUniform1f(shaderManager->u_TimeUniformLoc, static_cast<float>(glfwGetTime()));
	}
}


std::unique_ptr<Application>& Application::GetInstance() {
    if (Application::s_Instance == nullptr) {
        Application::s_Instance = std::unique_ptr<Application>(new Application);
    }
    return Application::s_Instance;
}