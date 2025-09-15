#pragma once

#include <glad/gl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "shader.h"
#include <glm/ext/vector_float3.hpp>

namespace RayTracer {
	Shader::Shader() {
	}

	Shader::~Shader() {
		deleteShaders();
	}

	void Shader::init() {
		createShaderProgram();
		m_shaders.resize(ShaderType::COUNT);
	}

	void Shader::useShader() const {
		glUseProgram(m_shaderProgram);
	}

	void Shader::attachShader(const char* shaderPath, ShaderType shaderType) {
		if (m_shaders[shaderType] != 0) {
			std::cout << "Shader " << shaderType << " already attached!" << std::endl;
			return;
		}

		std::string shaderCode;
		std::ifstream shaderFile;

		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			// open files
			shaderFile.open(shaderPath);
			std::stringstream shaderStream;
			// read file's buffer contents into streams
			shaderStream << shaderFile.rdbuf();
			// close file handlers
			shaderFile.close();
			// convert stream into string
			shaderCode = shaderStream.str();
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}

		m_shaders[shaderType] = glCreateShader(
			shaderType == VERTEX_SHADER               ? GL_VERTEX_SHADER        : 
			shaderType == FRAGMENT_SHADER             ? GL_FRAGMENT_SHADER      : 
			shaderType == COMPUTE_SHADER              ? GL_COMPUTE_SHADER       :
			shaderType == GEOMETRY_SHADER             ? GL_GEOMETRY_SHADER      : 
			shaderType == TESSELLATION_CONTROL_SHADER ? GL_TESS_CONTROL_SHADER  : GL_TESS_EVALUATION_SHADER);

		const char* shaderCodeNew = shaderCode.c_str();

		glShaderSource(m_shaders[shaderType], 1, &shaderCodeNew, NULL);
		glCompileShader(m_shaders[shaderType]);
		glAttachShader(m_shaderProgram, m_shaders[shaderType]);
	}

	void Shader::linkProgram() {
		glLinkProgram(m_shaderProgram);
	}

	GLuint Shader::getShaderProgam() const {
		return m_shaderProgram;
	}

	void Shader::dispatchCompute(glm::vec3 numGroups) {
		glDispatchCompute(static_cast<GLuint>(numGroups.x), static_cast<GLuint>(numGroups.y), static_cast<GLuint>(numGroups.z));
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void Shader::bindImageTexture(GLuint binding, GLuint texture, GLenum access, GLenum format) {
		glBindImageTexture(binding, texture, 0, GL_FALSE, 0, access, format);
	}

	void Shader::createShaderProgram() {
		m_shaderProgram = glCreateProgram();
	}

	void Shader::deleteShaders() {
		for (auto shader : m_shaders) {
			glDeleteShader(shader);
		}
	}
}
