#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "renderer.h"

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
    };

    class RayTracer {
    public:
        RayTracer();
        std::vector<glm::vec3> run(int bounceLimit, Renderer* renderer);

    private:
        glm::vec3 traceRay(Ray& ray, const std::vector<Sphere>& spheres, bool& canBounce);
        bool isRayIntersectSphere(const Ray& ray, const Sphere& sphere, float& closestIntersection);

    private:
        std::vector<Sphere> m_spheres;
    };
}
