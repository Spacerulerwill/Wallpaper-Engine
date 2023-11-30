#include <core/Application.hpp>
#include <ShObjIdl.h>
#include <system_error>
#include <unordered_map>
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <util/Log.hpp>

// Get the directory of the currently active wallpaper in Windows 10
wchar_t* GetWallpaper() {
    wchar_t buf[512];
    SystemParametersInfoW(SPI_GETDESKWALLPAPER, 512, buf, 0);
    return &buf[0];
}

// Set the Windows 10 wallpaper to the path specified
void SetWallpaper(wchar_t* path)
{
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, static_cast<void*>(path), SPIF_UPDATEINIFILE);
}

// open a Windows 10 file dialog window
bool openFileDialog(std::string* sFilePath)
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

RECT GetDesktopRect() {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    return desktop;
}

HWND GetWallpaperHwnd()
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

Application::Application() : p_WallpaperDir(GetWallpaper())
{

}

void Application::Run()
{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialise GLFW!");
    }

    // Create windows
    RECT desktopRect = GetDesktopRect();
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    p_WallpaperWindow = std::make_unique<Window>(1920, 1080, "");
    HWND desktopWallaperHwnd = GetWallpaperHwnd();
    HWND wallpaperWindowHwnd = glfwGetWin32Window(p_WallpaperWindow->GetWindow());
    SetParent(wallpaperWindowHwnd, desktopWallaperHwnd);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    p_ImGUIWindow = std::make_unique<Window>(400, 400, "Wallpaper Engine");

    p_WallpaperWindow->Bind();

    // initialise GLAD
    // ---------------
    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Failed to intialise GLAD!");
    }

    p_ShaderManager = std::make_unique<ShaderManager>();
    p_ShaderManager->SetShader("res/default.frag", p_WallpaperWindow->GetDimensions());

    // ImGui context creationg and start up
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL; // disable imgui.ini
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(p_ImGUIWindow->GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // application loop
    // ---------------
    while (!p_ImGUIWindow->ShouldClose()) {
        // render wallpaper window
        // -----------------------
        p_WallpaperWindow->Bind();
        SendDefaultUniforms();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // update non default uniforms
        std::unordered_map<std::string, Uniform>map = p_ShaderManager->getUniformMap();
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

        glDrawArrays(GL_TRIANGLES, 0, 6);
        p_WallpaperWindow->SwapBuffers();

        CheckImGUIButtons();

        // render ImGUI window
        // -----------------------
        p_ImGUIWindow->Bind();
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
        ImGui::Text(p_ShaderManager->getFragmentPath().c_str());

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
        p_ImGUIWindow->SwapBuffers();
        glfwPollEvents();

    }
    SetWallpaper(p_WallpaperDir);
    glDeleteVertexArrays(1, &vao);
}


void Application::CheckImGUIButtons()
{
    if (m_isLoadShaderButtonPressed) {
        std::string newPath = std::string();
        bool success = openFileDialog(&newPath);

        if (success) {
            p_ShaderManager->SetShader(newPath, p_WallpaperWindow->GetDimensions());
        }
        else {
            LOG_ERROR("Failed to load shader");
        }
    }
}

void Application::SendDefaultUniforms()
{
    // send mouse uniform if needed
    if (p_ShaderManager->m_MouseUniformLoc != -1) {
        POINT p;
        if (GetCursorPos(&p))
        {
            glUniform2f(p_ShaderManager->m_MouseUniformLoc, static_cast<float>(p.x), static_cast<float>(p.y));
        }
    }

    // send time if needed
    if (p_ShaderManager->m_TimeUniformLoc != -1) {
        glUniform1f(p_ShaderManager->m_TimeUniformLoc, static_cast<float>(glfwGetTime()));
    }
}