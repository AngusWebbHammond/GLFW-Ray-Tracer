#pragma once

#include <glad/gl.h>
#include <iostream>

#include "application.h"
#include <cstdlib>

namespace RayTracer {
	Application::Application() {
	}

	Application::~Application() {
		glfwTerminate();
		UI::cleanupImGui();
	}

	void Application::init(GLuint width, GLuint height) {
		createWindow(width, height);
		UI::initImGui(m_window);
		m_renderer.init(m_window);
		m_rayTracer.init();
		m_bounces = 12;
	}

	void Application::run() {
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();

			UI::createImGuiFrame();
			UI::createImGuiWindows(&m_renderer);
			UI::createImGuiPropertiesPanel(m_rayTracer);

			float timeStart = glfwGetTime();
#if 1
			m_rayTracer.run(m_bounces, &m_renderer);
#else
			m_renderer.render(m_rayTracer.run(m_bounces, &m_renderer));
#endif

			float timeEnd = glfwGetTime();

			float elapsedTime = timeEnd - timeStart;

			ImGui::Begin("Stats");
			ImGui::Text("Frame Time: %.3f ms", elapsedTime * 1000);
			ImGui::Text("FPS: %.f", 1 / elapsedTime);

			ImGui::Separator();

			ImGui::Checkbox("Accumulate", &m_rayTracer.m_accumilate);
			ImGui::Text("Frames: %.i", m_rayTracer.m_frames);
			ImGui::InputInt("Bounces", &m_bounces);

			ImGui::End();

			UI::renderImGui();
			glfwSwapBuffers(m_window);
		}
	}

	void Application::createWindow(GLuint width, GLuint height) {
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(width, height, "Ray Tracer GLFW", NULL, NULL);
		glfwMakeContextCurrent(m_window);

		if (!gladLoadGL(glfwGetProcAddress)) {
			std::cerr << "Failed to initialize GLAD!" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		std::cout << "GLAD initialized successfully!" << std::endl;
		std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

		glViewport(0, 0, width, height);
	}
}
