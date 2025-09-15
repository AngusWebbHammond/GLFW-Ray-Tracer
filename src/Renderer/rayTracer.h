#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "renderer.h"
#include "../Shader/shader.h"


namespace RayTracer {
	struct alignas(16) Material {
		glm::vec3 materialColour;
		float reflectivness;

		float emissiveStrength;
		glm::vec3 emissionColour;

		Material(glm::vec3 colour);
	};

	struct alignas(16) Sphere {
		glm::vec3 centre;
		float radius;
		Material material;
	};

	struct Ray {
		glm::vec3 origin;
		glm::vec3 direction;
	};

	struct Camera {
		glm::vec3 location;
		float fov;
	};

	struct HitSphere {
		glm::vec3 hitPoint;
		glm::vec3 hitColour;
		glm::vec3 hitNormal;

		Material hitMaterial;

		glm::vec3 hitLight;
	};

	struct Random {
		using resultType = std::uint32_t;

		Random() = default;
		Random(std::uint32_t seed);

		static constexpr resultType min() { return 0; }
		static constexpr resultType max() { return UINT32_MAX; }

		resultType operator()() {
			return getRandomFloat();
		}

	private:
		resultType m_randomNumber;
		std::uint32_t getRandomFloat();
	};

	class RayTracer {

	public:
		RayTracer();
		void init();

		std::vector<glm::vec3> run(int bounceLimit, Renderer* renderer);

	private:
		glm::vec3 traceRay(Ray& ray, const std::vector<Sphere>& spheres, int bounceLimit);
		bool isRayIntersectSphere(const Ray& ray, const Sphere& sphere, float& closestIntersection);
		glm::vec3 getRandomOnUnitSphere();

	public:
		std::vector<Sphere> m_spheres;
		bool m_accumilate;
		int m_frames;
		glm::vec3 m_background;

	private:
		std::vector<glm::vec3> m_accumilateFrameBuffer;
		Shader m_computeShader;

		GLuint m_sphereSSBO;
		GLuint m_paramsUBO;

		struct ParamsUBO {
			alignas(16) glm::vec2 info;
		};

		ParamsUBO m_params;
	};
}
