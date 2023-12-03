#include <core/Application.hpp>
#include <stdexcept>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <util/Log.hpp>
#include <util/OS.hpp>

#include <any>

Application::Application() : pWallpaperDir(GetWallpaper())
{

}

void Application::Run()
{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialise GLFW!");
    }

    /*
    Create 2 windows. The first being a window the size of the desktop which is attatched
    to the desktop wallpaper which cannot be moved or interacted with at all apart from by
    the second window, which is smaller movable non resizable control menu.
    */
    RECT desktopRect = GetDesktopRect();
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    pWallpaperWindow = std::make_unique<Window>(1920, 1080, "");
    HWND desktopWallaperHwnd = GetWallpaperHwnd();
    HWND wallpaperWindowHwnd = glfwGetWin32Window(pWallpaperWindow->GetWindow());
    SetParent(wallpaperWindowHwnd, desktopWallaperHwnd);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    pImGUIWindow = std::make_unique<Window>(400, 400, "Wallpaper Engine");

    pWallpaperWindow->Bind();

    // Initialise GLAD and load OpenGL function pointers
    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Failed to intialise GLAD!");
    }

    // Create our shader manager and set it to use the default shader
    pShaderManager = std::make_unique<ShaderManager>();
    pShaderManager->SetFragmentShader("res/default.frag", pWallpaperWindow->GetDimensions());
    ImGui::CreateContext();

    /*
    ImGUI setup. Tell ImGUI to not have a imgui.ini file, so that the position and size of widgets
    will not perisist between program executions.
    */
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(pImGUIWindow->GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    /*
    We are rendering a fullscreen quad to our wallpaper window by hardcoding the vertices for it in the
    vertex shader. This means that no VBO or any buffer objects are required however OpenGL requires there
    to be a non default VAO bound always when we want to draw, so here we create one and never use it again
    to enable us to draw our fullscreen quad.
    */
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /*
    Main application loop, we loop whilst the ImGUI window is open as the user should not be able to interact
    with the wallpaper window
    */
    while (!pImGUIWindow->ShouldClose()) {
        pWallpaperWindow->Bind();
        UpdateUniforms();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        pWallpaperWindow->SwapBuffers();

        ProcessImGUI();

        // render ImGUI window
        // -----------------------
        pImGUIWindow->Bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawImGUIControlMenu();

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        pImGUIWindow->SwapBuffers();
        glfwPollEvents();

    }
    // Final cleanup, we reset the users wallpaper to what it was before and delete our VAO
    SetWallpaper(pWallpaperDir);
    glDeleteVertexArrays(1, &vao);
}

void Application::ProcessImGUI()
{
    if (mIsLoadShaderButtonPressed) {
        std::string newPath = std::string();
        bool success = openFileDialog(&newPath);

        if (success) {
            pShaderManager->SetFragmentShader(newPath, pWallpaperWindow->GetDimensions());
        }
        else {
            LOG_ERROR("Failed to load shader");
        }
    }
}

void Application::DrawImGUIControlMenu()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Control Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    mIsLoadShaderButtonPressed = ImGui::Button("Load Shader");

    for (auto it = pShaderManager->mUniforms.begin(); it != pShaderManager->mUniforms.end(); ++it) {
        const char* name = it->first.c_str();
        unsigned int loc = it->second.location;
        GLenum type = it->second.type;;
        switch (type) {
        case GL_INT: {
            ImGui::SliderInt(name, std::any_cast<GLint*> (it->second.var), 0, 100);
            break;
        }
        case GL_INT_VEC2: {
            ImGui::SliderInt2(name, std::any_cast<GLint*> (it->second.var), 0, 100);
            break;
        }
        case GL_INT_VEC3: {
            ImGui::SliderInt3(name, std::any_cast<GLint*> (it->second.var), 0, 100);
            break;
        }
        case GL_INT_VEC4: {
            ImGui::SliderInt4(name, std::any_cast<GLint*> (it->second.var), 0, 100);
            break;
        }
        case GL_FLOAT: {
            ImGui::SliderFloat(name, std::any_cast<GLfloat*>(it->second.var), 0.0f, 10.0f);
            break;
        }
        case GL_FLOAT_VEC2: {
            ImGui::SliderFloat2(name, std::any_cast<GLfloat*>(it->second.var), 0.0f, 10.0f);
            break;
        }
        case GL_FLOAT_VEC3: {
            ImGui::SliderFloat3(name, std::any_cast<GLfloat*>(it->second.var), 0.0f, 10.0f);
            break;
        }
        case GL_FLOAT_VEC4: {
            ImGui::SliderFloat4(name, std::any_cast<GLfloat*>(it->second.var), 0.0f, 10.0f);
            break;
        }
        case GL_BOOL: {
            ImGui::Checkbox(name, reinterpret_cast<bool*>(std::any_cast<int*>(it->second.var)));
        }
        }
    }
}

void Application::UpdateUniforms()
{
    // First part is to send builtin uniforms
    // Update mouse uniform only if required
    if (pShaderManager->mBuiltinUniformsLocations.mousePos != GL_INVALID_INDEX) {
        POINT p;
        if (GetCursorPos(&p))
        {
            glUniform2f(pShaderManager->mBuiltinUniformsLocations.mousePos, static_cast<float>(p.x), static_cast<float>(p.y));
        }
    }

    // Update time uniform only if required
    if (pShaderManager->mBuiltinUniformsLocations.time != GL_INVALID_INDEX) {
        glUniform1f(pShaderManager->mBuiltinUniformsLocations.time, static_cast<float>(glfwGetTime()));
    }
    // Second part is to update all the uniforms that aren't builtins
    for (auto it = pShaderManager->mUniforms.begin(); it != pShaderManager->mUniforms.end(); ++it) {
        GLint loc = it->second.location;
        GLenum type = it->second.type;

        switch (type) {
        case GL_FLOAT: {
            glUniform1f(loc, *std::any_cast<GLfloat*>(it->second.var));
            break;
        }
        case GL_FLOAT_VEC2: {
            glUniform2fv(loc, 1, std::any_cast<GLfloat*>(it->second.var));
            break;
        }
        case GL_FLOAT_VEC3: {
            glUniform3fv(loc, 1, std::any_cast<GLfloat*>(it->second.var));
            break;
        }
        case GL_FLOAT_VEC4: {
            glUniform4fv(loc, 1, std::any_cast<GLfloat*>(it->second.var));
            break;
        }
        case GL_INT: {
            glUniform1i(loc, *std::any_cast<GLint*>(it->second.var));
            break;
        }
        case GL_INT_VEC2: {
            glUniform2iv(loc, 1, std::any_cast<GLint*>(it->second.var));
            break;
        }
        case GL_INT_VEC3: {
            glUniform3iv(loc, 1, std::any_cast<GLint*>(it->second.var));
            break;
        }
        case GL_INT_VEC4: {
            glUniform4iv(loc, 1, std::any_cast<GLint*>(it->second.var));
            break;
        }
        case GL_BOOL: {
            glUniform1i(loc, *std::any_cast<GLint*>(it->second.var));
            break;
        }
        }
    }
}