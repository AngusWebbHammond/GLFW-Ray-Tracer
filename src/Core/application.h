#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glad/gl.h>

namespace RayTracer {
    class Application {
    public:
        Application(GLuint width, GLuint height);
        ~Application();

        void run();
    private:
        GLFWwindow* m_window;
    };
}
