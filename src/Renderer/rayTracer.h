#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "renderer.h"

namespace RayTracer {
    struct Sphere {
        glm::vec3 centre;
        float radius;
        glm::vec3 colour;
    };

    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    struct Camera {
        glm::vec3 location;
        float fov;
    };

    class RayTracer {
    public:
        RayTracer();
        std::vector<glm::vec3> run(int bounceLimit, Renderer* renderer);

    private:
        glm::vec3 traceRay(Ray& ray, const std::vector<Sphere>& spheres);
        bool isRayIntersectSphere(const Ray& ray, const Sphere& sphere, float& closestIntersection);

    private:
        std::vector<Sphere> m_spheres;
    };
}
