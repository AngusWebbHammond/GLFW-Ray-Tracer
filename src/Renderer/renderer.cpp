#pragma once

#include "renderer.h"

namespace RayTracer {
    Renderer::Renderer()
    {
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::init(GLFWwindow* window)
    {
        m_window = window;
        createOpenGLTexture();
    }

    void Renderer::render(std::vector<glm::vec3> frameBuffer)
    {
        updateOpenGLTexture(frameBuffer);
    }

    GLuint Renderer::getTexture()
    {
        return m_texture;
    }

    FrameBufferSettings Renderer::getFrameBufferSize()
    {
        return FrameBufferSettings{ m_windowWidth, m_windowHeight };
    }

    void Renderer::setWidthAndHeight(int width, int height)
    {
        m_windowWidth = width;
        m_windowHeight = height;
    }

    void Renderer::createOpenGLTexture()
    {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Renderer::updateOpenGLTexture(std::vector<glm::vec3> framebuffer)
    {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_windowWidth, m_windowHeight, 0, GL_RGB, GL_FLOAT, framebuffer.data());
    }
}
