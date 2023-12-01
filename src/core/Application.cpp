#include <core/Application.hpp>
#include <system_error>
#include <unordered_map>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <util/Log.hpp>
#include <util/OS.hpp>

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
    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Failed to intialise GLAD!");
    }

    p_ShaderManager = std::make_unique<ShaderManager>();
    p_ShaderManager->SetFragmentShader("res/default.frag", p_WallpaperWindow->GetDimensions());
    ImGui::CreateContext();

    // ImGui context creationg and start up
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL; // disable imgui.ini
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(p_ImGUIWindow->GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // application loop
    while (!p_ImGUIWindow->ShouldClose()) {
        // render wallpaper window
        p_WallpaperWindow->Bind();
        SendDefaultUniforms();
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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
            p_ShaderManager->SetFragmentShader(newPath, p_WallpaperWindow->GetDimensions());
        }
        else {
            LOG_ERROR("Failed to load shader");
        }
    }
}

void Application::SendDefaultUniforms()
{
    // send mouse uniform if needed
    if (p_ShaderManager->BuiltinUniformsLocations.mousePos != -1) {
        POINT p;
        if (GetCursorPos(&p))
        {
            glUniform2f(p_ShaderManager->BuiltinUniformsLocations.mousePos, static_cast<float>(p.x), static_cast<float>(p.y));
        }
    }

    // send time if needed
    if (p_ShaderManager->BuiltinUniformsLocations.time != -1) {
        glUniform1f(p_ShaderManager->BuiltinUniformsLocations.time, static_cast<float>(glfwGetTime()));
    }
}