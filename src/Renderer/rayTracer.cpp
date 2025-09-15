#pragma once

#include <iostream>
#include <numeric>
#include <algorithm>
#include <execution>
#include <random>
#include <filesystem>

#include "rayTracer.h"
#include "../Shader/shader.h"

namespace RayTracer {
	RayTracer::RayTracer() {
	}

	void RayTracer::init() {
		Material material1 = Material({ 1.0f, 1.0f, 1.0f });
		Material material2 = Material({ 1.0f, 1.0f, 1.0f });
		Material material3 = Material({ 1.0f, 0.9f, 0.4f });
		Material material4 = Material({ 1.0f, 0.3f, 0.0f });
		Material material5 = Material({ 0.5f, 1.0f, 1.0f });

		material1.emissionColour = glm::vec3(1.0f);
		material1.emissiveStrength = 2.3f;

		material2.reflectivness = 1.0f;

		m_spheres = {
			Sphere({ { 0.0f, 23.9f, -3.0f }, 9.3f, material1 }),
			Sphere({ { 0.0f, 0.0f, -2.5f }, 1.0f, material2 }),
			Sphere({ { 3.0f, 0.0f, -3.0f }, 1.25f, material3 }),
			Sphere({ { -3.0f, 0.0f, -3.0f }, 1.25f, material4 }),
			Sphere({ { 0.0f, -20.0f, -3.0f }, 19.0f, material5 }),
		};

		m_accumilate = false;
		m_frames = 1;
		m_background = glm::vec3(0.5f);

		m_computeShader.init();
		m_computeShader.attachShader((std::filesystem::path(PROJECT_DIR) / "assets" / "shaders" / "computeShader.glsl").string().c_str(), COMPUTE_SHADER);
		m_computeShader.linkProgram();

		glGenBuffers(1, &m_sphereSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_sphereSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_spheres.size() * sizeof(Sphere), m_spheres.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_sphereSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		m_params.info.x = m_spheres.size();
		m_params.info.y = 0;

		std::cout << "Sphere count: " << m_params.info.x << std::endl;

		std::cout << "Sphere size: " << sizeof(Sphere) << std::endl;

		glGenBuffers(1, &m_paramsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_paramsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ParamsUBO), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_paramsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ParamsUBO), &m_params);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	std::vector<glm::vec3> RayTracer::run(int bounceLimit, Renderer* renderer) {
		FrameBufferSettings frameBufferSize = renderer->getFrameBufferSize();
		std::vector<glm::vec3> frameBuffer(frameBufferSize.width * frameBufferSize.height);

		if (m_accumilateFrameBuffer.empty() || frameBuffer.size() != m_accumilateFrameBuffer.size()) {
			m_accumilateFrameBuffer.resize(frameBufferSize.width * frameBufferSize.height, glm::vec3(0.0f));
		}

		Camera camera;
		camera.fov = 45.0f;
		camera.location = glm::vec3(0.0f, 0.0f, -10.0f);
		int fbHeight = frameBufferSize.height;
		int fbWidth = frameBufferSize.width;

		float rayFactor = glm::tan(glm::radians(camera.fov) / 2.0f);
		float aspectRatio = frameBufferSize.width / static_cast<float>(frameBufferSize.height);

		float rayFactorAR = rayFactor * aspectRatio;

		if (m_accumilate)
			m_frames++;
		else {
			m_frames = 1;
		}

#define COMPUTE_SHADER
// #define SINGLE_THREAD
#ifdef COMPUTE_SHADER
		frameBuffer.resize(fbHeight * fbWidth, glm::vec3(0.0f));

		renderer->updateOpenGLTexture(frameBuffer);

		m_computeShader.useShader();
		m_computeShader.bindImageTexture(0, renderer->getTexture(), GL_WRITE_ONLY, GL_RGBA32F);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_sphereSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_spheres.size() * sizeof(Sphere), m_spheres.data(), GL_STATIC_DRAW);
		m_computeShader.dispatchCompute(glm::vec3((fbWidth + 16 - 1) / 16, (fbHeight + 16 - 1) / 16, 1));

		glBindBuffer(GL_UNIFORM_BUFFER, m_paramsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ParamsUBO), &m_params);

		m_params.info.y++;

#else
#ifdef SINGLE_THREAD
		for (int i = 0; i < fbHeight; i++) {
			for (int j = 0; j < fbWidth; j++) {
				Ray ray;
				ray.origin = camera.location;
				ray.direction = glm::normalize(glm::vec3(
					(2.0f * (j + 0.5f) / frameBufferSize.width - 1.0f) * rayFactorAR,
					(1.0f - 2.0f * (i + 0.5f) / frameBufferSize.height) * rayFactor,
					1.0f));

				glm::vec3 colour = traceRay(ray, m_spheres, bounceLimit);

				int pixelIndex = i * frameBufferSize.width + j;

				if (m_accumilate) {
					m_accumilateFrameBuffer[pixelIndex] += colour;
					frameBuffer[pixelIndex] = m_accumilateFrameBuffer[pixelIndex] / glm::vec3(m_frames);
				}

				else {
					m_accumilateFrameBuffer[pixelIndex] = glm::vec3(0.0f);
					frameBuffer[pixelIndex] = colour;
				}
			}
		}
#else
		std::vector<int> indices(fbWidth * fbHeight);
		std::iota(indices.begin(), indices.end(), 0);

		// Thanks to The Cherno for the for_each + lambda idea for parallelism
		std::for_each(
			std::execution::par,
			indices.begin(),
			indices.end(),
			[&](int idx) {
				int i = idx / fbWidth;
				int j = idx % fbWidth;

				Ray ray;
				ray.origin = camera.location;
				ray.direction = glm::normalize(glm::vec3(
					(2.0f * (j + 0.5f) / frameBufferSize.width - 1.0f) * rayFactorAR,
					(1.0f - 2.0f * (i + 0.5f) / frameBufferSize.height) * rayFactor,
					1.0f));

				glm::vec3 colour = traceRay(ray, m_spheres, bounceLimit);

				int pixelIndex = i * frameBufferSize.width + j;

				if (m_accumilate) {
					m_accumilateFrameBuffer[pixelIndex] += colour;
					frameBuffer[pixelIndex] =
						m_accumilateFrameBuffer[pixelIndex] / glm::vec3(m_frames);
				}
				else {
					m_accumilateFrameBuffer[pixelIndex] = glm::vec3(0.0f);
					frameBuffer[pixelIndex] = colour;
				}
			});
#endif
#endif

		return frameBuffer;
	}

	glm::vec3 RayTracer::traceRay(Ray& ray, const std::vector<Sphere>& spheres, int bounceLimit) {
		glm::vec3 colour(0.0f);
		glm::vec3 attenuation(1.0f);

		HitSphere hitSphere = HitSphere({ glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material({ { 0.0f, 0.0f, 0.0f } }), glm::vec3(0.0f) });

		for (int t = 0; t < bounceLimit; t++) {

			glm::vec3 bounceColour{ 0.0f };

			float closestIntersection = std::numeric_limits<float>::max();
			for (auto& sphere : spheres) {
				float intersection;
				if (isRayIntersectSphere(ray, sphere, intersection)) {
					if (intersection < closestIntersection) {
						closestIntersection = intersection;

						hitSphere.hitPoint = ray.origin + ray.direction * intersection;
						hitSphere.hitNormal = glm::normalize(hitSphere.hitPoint - sphere.centre);
						hitSphere.hitMaterial = sphere.material;
					}
				}
			}

			if (closestIntersection < std::numeric_limits<float>::max()) {


				hitSphere.hitLight += hitSphere.hitMaterial.emissiveStrength * hitSphere.hitMaterial.emissionColour * attenuation;

				ray.origin = hitSphere.hitPoint + 0.001f * hitSphere.hitNormal;

				glm::vec3 randomNum = getRandomOnUnitSphere();
				if (glm::dot(randomNum, hitSphere.hitNormal) < 0) {
					// randomNum and hitNormal are both unit vectors, so this does not need to be normalized
					randomNum = glm::reflect(randomNum, hitSphere.hitNormal);
				}

				// Combines the specular and diffuse bounces
				// Uses Lamberts cosine law for diffuse bounces (favours bounces closer to the normal)
				ray.direction = glm::normalize((1 - hitSphere.hitMaterial.reflectivness) * glm::normalize(hitSphere.hitNormal + randomNum) + hitSphere.hitMaterial.reflectivness * glm::reflect(ray.direction, hitSphere.hitNormal));

				colour += hitSphere.hitLight * hitSphere.hitColour;

				hitSphere.hitColour *= hitSphere.hitMaterial.materialColour;
			}

			else {
				if (t == 0) {
					return m_background;
				}

				colour += m_background * hitSphere.hitColour * attenuation;
				return colour;
			}

			attenuation *= 0.75f;
		}
		return colour;
	}

	bool RayTracer::isRayIntersectSphere(const Ray& ray, const Sphere& sphere, float& intersection) {
		// Assuming ray direction is normalised
		// Causes aTerm = 1
		float bTerm = glm::dot(ray.origin - sphere.centre, ray.direction);
		float cTerm = glm::dot(ray.origin - sphere.centre, ray.origin - sphere.centre) - sphere.radius * sphere.radius;

		float determinant = bTerm * bTerm - cTerm;

		if (determinant < 0) {
			return false;
		}

		float determinantSqrt = glm::sqrt(determinant);

		float term0 = -bTerm - determinantSqrt;
		float term1 = -bTerm + determinantSqrt;

		if (term0 > 0.001f) {
			intersection = term0;
			return true;
		}
		if (term1 > 0.001f) {
			intersection = term1;
			return true;
		}
		return false;
	}

	glm::vec3 RayTracer::getRandomOnUnitSphere() {
		// Thanks to The Cherno https://www.youtube.com/watch?v=1KTgc2SEt50&list=PLlrATfBNZ98edc5GshdBtREv5asFW3yXl&index=12 for the Thread Local idea
		thread_local Random rng(123456789 + std::hash<std::thread::id>()(std::this_thread::get_id()));
		thread_local std::normal_distribution<float> normalDistribution(0.0f, 1.0f);

		// Thanks to Sebastian Lauge https://www.youtube.com/watch?v=Qz0KTGYJtUk&t=545s for the uniform random on unit sphere idea
		float x = normalDistribution(rng);
		float y = normalDistribution(rng);
		float z = normalDistribution(rng);

		return glm::normalize(glm::vec3(x, y, z));
	}

	Random::Random(std::uint32_t seed) {
		m_randomNumber = seed;
	}

	std::uint32_t Random::getRandomFloat() {
		uint32_t result = m_randomNumber;
		result ^= result << 13;
		result ^= result >> 17;
		result ^= result << 5;
		m_randomNumber = result;

		return result;
	}

	Material::Material(glm::vec3 colour) {
		materialColour = colour;
		reflectivness = 0.0f;

		emissiveStrength = 0.0f;
		emissionColour = glm::vec3(0.0f);
	}
}
