#include <opengl/Window.hpp>
#include <stdexcept>

Window::Window(unsigned int width, unsigned int height, const char* name) {
    // Create our window, and add its callbacks
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    p_Window = glfwCreateWindow(width, height, name, NULL, NULL);

    if (p_Window == NULL)
    {
        throw std::runtime_error("Failed to create GLFW Window");
    }
}

Window::~Window() {
    glfwDestroyWindow(p_Window);
}

void Window::Bind() const {
    glfwMakeContextCurrent(p_Window);
}

void Window::Unbind() const {
    glfwMakeContextCurrent(NULL);
}

void Window::SetShouldClose(int value) {
    glfwSetWindowShouldClose(p_Window, value);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(p_Window);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(p_Window);
}

GLFWwindow* Window::GetWindow() const {
    return p_Window;
}

WindowDimensions Window::GetDimensions() const {
    WindowDimensions dim{};
    glfwGetWindowSize(p_Window, &dim.width, &dim.height);
    return dim;
}
