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