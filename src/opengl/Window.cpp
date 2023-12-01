#include <opengl/Window.hpp>
#include <stdexcept>

Window::Window(unsigned int width, unsigned int height, const char* name) {
    // Create our window, and add its callbacks
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    pWindow = glfwCreateWindow(width, height, name, NULL, NULL);

    if (pWindow == NULL)
    {
        throw std::runtime_error("Failed to create GLFW Window");
    }
}

Window::~Window() {
    glfwDestroyWindow(pWindow);
}

void Window::Bind() const {
    glfwMakeContextCurrent(pWindow);
}

void Window::Unbind() const {
    glfwMakeContextCurrent(NULL);
}

void Window::SetShouldClose(int value) {
    glfwSetWindowShouldClose(pWindow, value);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(pWindow);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(pWindow);
}

GLFWwindow* Window::GetWindow() const {
    return pWindow;
}

WindowDimensions Window::GetDimensions() const {
    WindowDimensions dim{};
    glfwGetWindowSize(pWindow, &dim.width, &dim.height);
    return dim;
}
