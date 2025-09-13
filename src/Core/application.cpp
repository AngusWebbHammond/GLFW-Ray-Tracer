#pragma once

#include "application.h"

namespace RayTracer {
    Application::Application(GLuint width, GLuint height)
    {
        createWindow(width, height);
        UI::initImGui(m_window);
        m_renderer.init(m_window);
    }

    Application::~Application()
    {
        glfwTerminate();
        UI::cleanupImGui();
    }

    void Application::run() {
        while (!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();

            UI::createImGuiFrame();
            UI::createImGuiWindows(&m_renderer);
            UI::createImGuiPropertiesPanel(m_rayTracer);

            float timeStart = glfwGetTime();

            m_renderer.render(m_rayTracer.run(3, &m_renderer));

            float timeEnd = glfwGetTime();

            float elapsedTime = timeEnd - timeStart;

            ImGui::Begin("Stats");
            ImGui::Text("Frame Time: %.3f ms", elapsedTime * 1000);
            ImGui::Text("FPS: %.f", 1 / elapsedTime);
            ImGui::Separator();
            ImGui::Checkbox("Accumulate", &m_rayTracer.m_accumilate);
            ImGui::Text("Frames: %.i", m_rayTracer.m_frames);
            ImGui::End();

            UI::renderImGui();
            glfwSwapBuffers(m_window);
        }
    }

    void Application::createWindow(GLuint width, GLuint height)
    {
        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, "Ray Tracer GLFW", NULL, NULL);
        glfwMakeContextCurrent(m_window);

        gladLoadGL(glfwGetProcAddress);
        glViewport(0, 0, width, height);
    }
}

