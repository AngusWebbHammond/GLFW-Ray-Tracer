#pragma once

#include <glad/gl.h>
#include <vector>
#include <glm/ext/vector_float3.hpp>

namespace RayTracer {
	enum ShaderType {
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		COMPUTE_SHADER,
		GEOMETRY_SHADER,
		TESSELLATION_CONTROL_SHADER,
		TESSELLATION_EVALUATION_SHADER,
		COUNT
	};

	class Shader {
	public:
		Shader();
		~Shader();

		void init();

		void useShader() const;
		void attachShader(const char* shaderPath, ShaderType shaderType);
		void linkProgram();
		GLuint getShaderProgam() const;

		void dispatchCompute(glm::vec3 numGroups);

		void bindImageTexture(GLuint binding, GLuint texture, GLenum access, GLenum format);

	private:
		void createShaderProgram();
		void deleteShaders();

	private:
		std::vector<GLuint> m_shaders;
		GLuint m_shaderProgram;
	};
}
