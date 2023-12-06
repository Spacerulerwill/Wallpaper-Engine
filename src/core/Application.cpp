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
    pWallpaperWindow = std::make_unique<Window>(desktopRect.right, desktopRect.bottom, "");
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
    pWallpaperManager = std::make_unique<WallpaperManager>();
    pWallpaperManager->TrySetWallpaper("res/default.wallpaper", pWallpaperWindow->GetDimensions());
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

        if (pWallpaperManager->hasWallpaper) {
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

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
            pWallpaperManager->TrySetWallpaper(newPath, pWallpaperWindow->GetDimensions());
        }
        else {
            LOG_ERROR("Failed to load shader");
        }
    }

    if (mIsUnloadShaderButtonPressed) {
        pWallpaperManager->hasWallpaper = false;
    }
}

void Application::DrawImGUIControlMenu()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    if (pWallpaperManager->mMetadata.name == "") {
        ImGui::Begin("Control Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    }
    else {
        ImGui::Begin(pWallpaperManager->mMetadata.name.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    }

    mIsLoadShaderButtonPressed = ImGui::Button("Load Shader");
    ImGui::SameLine();
    mIsUnloadShaderButtonPressed = ImGui::Button("Unload Shader");

    // Controls for integer uniforms
    for (auto it = pWallpaperManager->mIntUniforms.begin(); it != pWallpaperManager->mIntUniforms.end(); ++it) {
        const char* name = it->first.c_str();
        Uniform<GLint>* uniform = &it->second;
        switch (uniform->elements.size()) {
        case 1:
            ImGui::SliderInt(name, uniform->elements.data(), 0, 100);
            break;
        case 2:
            ImGui::SliderInt2(name, uniform->elements.data(), 0, 100);
            break;
        case 3:
            ImGui::SliderInt3(name, uniform->elements.data(), 0, 100);
            break;
        case 4:
            ImGui::SliderInt4(name, uniform->elements.data(), 0, 100);
            break;
        }
    }

    // Slides for float uniforms
    for (auto it = pWallpaperManager->mFloatUniforms.begin(); it != pWallpaperManager->mFloatUniforms.end(); ++it) {
        const char* name = it->first.c_str();
        Uniform<GLfloat>* uniform = &it->second;
        switch (uniform->elements.size()) {
        case 1:
            ImGui::SliderFloat(name, uniform->elements.data(), 0.0f, 100.0f);
            break;
        case 2:
            ImGui::SliderFloat2(name, uniform->elements.data(), 0.0f, 100.0f);
            break;
        case 3:
            ImGui::SliderFloat3(name, uniform->elements.data(), 0.0f, 100.0f);
            break;
        case 4:
            ImGui::SliderFloat4(name, uniform->elements.data(), 0.0f, 100.0f);
            break;
        }
    }

    // Checkboxes for bool uniforms
    for (auto it = pWallpaperManager->mBoolUniforms.begin(); it != pWallpaperManager->mBoolUniforms.end(); ++it) {
        const char* name = it->first.c_str();
        Uniform<GLboolean>* uniform = &it->second;
        ImGui::Checkbox(name, reinterpret_cast<bool*>(uniform->elements.data())); // TODO: Check this is okay?
    }
}

void Application::UpdateUniforms()
{
    // First part is to send builtin uniforms
    // Update mouse uniform only if required
    if (pWallpaperManager->mBuiltinUniformsLocations.mousePos != GL_INVALID_INDEX) {
        POINT p;
        if (GetCursorPos(&p))
        {
            glUniform2f(pWallpaperManager->mBuiltinUniformsLocations.mousePos, static_cast<float>(p.x), static_cast<float>(p.y));
        }
    }

    // Update time uniform only if required
    if (pWallpaperManager->mBuiltinUniformsLocations.time != GL_INVALID_INDEX) {
        glUniform1f(pWallpaperManager->mBuiltinUniformsLocations.time, static_cast<float>(glfwGetTime()));
    }

    // Second part is to update all the uniforms that aren't builtins
    for (auto it = pWallpaperManager->mIntUniforms.begin(); it != pWallpaperManager->mIntUniforms.end(); ++it) {
        Uniform<GLint> uniform = it->second;
        switch (uniform.elements.size()) {
        case 1:
            glUniform1iv(uniform.location, 1, uniform.elements.data());
            break;
        case 2:
            glUniform2iv(uniform.location, 1, uniform.elements.data());
            break;
        case 3:
            glUniform3iv(uniform.location, 1, uniform.elements.data());
            break;
        case 4:
            glUniform4iv(uniform.location, 1, uniform.elements.data());
            break;
        }
    }

    for (auto it = pWallpaperManager->mFloatUniforms.begin(); it != pWallpaperManager->mFloatUniforms.end(); ++it) {
        Uniform<GLfloat> uniform = it->second;
        switch (uniform.elements.size()) {
        case 1:
            glUniform1fv(uniform.location, 1, uniform.elements.data());
            break;
        case 2:
            glUniform2fv(uniform.location, 1, uniform.elements.data());
            break;
        case 3:
            glUniform3fv(uniform.location, 1, uniform.elements.data());
            break;
        case 4:
            glUniform4fv(uniform.location, 1, uniform.elements.data());
            break;
        }
    }

    for (auto it = pWallpaperManager->mBoolUniforms.begin(); it != pWallpaperManager->mBoolUniforms.end(); ++it) {
        Uniform<GLboolean> uniform = it->second;
        glUniform1i(uniform.location, static_cast<GLint>(uniform.elements.at(0)));
    }
}