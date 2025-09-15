#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glad/gl.h>

#include "ui.h"
#include "../Renderer/renderer.h"
#include "../Renderer/rayTracer.h"

namespace RayTracer {
	class Application {
	public:
		Application();
		~Application();

		void init(GLuint width, GLuint height);
		void run();

	private:
		void createWindow(GLuint width, GLuint height);

	private:
		GLFWwindow* m_window;
		Renderer m_renderer;
		RayTracer m_rayTracer;
		int m_bounces;
	};
}
