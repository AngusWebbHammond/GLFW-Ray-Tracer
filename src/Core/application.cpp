#pragma once

#include "application.h"

RayTracer::Application::Application(GLuint width, GLuint height)
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

RayTracer::Application::~Application()
{
    glfwTerminate();
}

void RayTracer::Application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(m_window);
    }
}
