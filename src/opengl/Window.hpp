#ifndef WINDOW_H
#define WINDOW_H

#include <gl.h>
#include <GLFW/glfw3.h>

struct WindowDimensions {
    int width;
    int height;
};

class Window {
private:
    GLFWwindow* pWindow = nullptr;
public:
    Window(unsigned int width, unsigned int height, const char* title);
    ~Window();
    void Bind() const;
    void Unbind() const;
    void SetShouldClose(int value);
    bool ShouldClose() const;
    void SwapBuffers();
    void SetHidden();
    void SetVisible();
    GLFWwindow* GetWindow() const;
    WindowDimensions GetDimensions() const;
};


#endif // !WINDOW_H