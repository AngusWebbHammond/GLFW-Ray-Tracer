#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "renderer.h"
#include <random>

namespace RayTracer {
    struct Material {
        glm::vec3 materialColour;

        float reflectivness;
        float metallicness;

        float emissiveStrength;
        glm::vec3 emissionColour;

        Material(glm::vec3 colour);
    };

    struct Sphere {
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

    class RayTracer {
    public:
        RayTracer();
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
        std::mt19937 m_random;
        std::uniform_real_distribution<double> m_uniformDistribution;

        std::vector<glm::vec3> m_accumilateFrameBuffer;
    };
}
