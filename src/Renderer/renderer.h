#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glad/gl.h>
#include <vector>
#include <glm/glm.hpp>

namespace RayTracer {
    struct FrameBufferSettings {
        int width, height;
    };

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void init(GLFWwindow* window);
        void render(std::vector<glm::vec3> frameBuffer);

        GLuint getTexture();
        FrameBufferSettings getFrameBufferSize();
        void setWidthAndHeight(int width, int height);

    private:
        void createOpenGLTexture();
        void updateOpenGLTexture(std::vector<glm::vec3> framebuffer);

    private:
        GLFWwindow* m_window;
        int m_windowWidth, m_windowHeight;

        GLuint m_texture;
    };
}
