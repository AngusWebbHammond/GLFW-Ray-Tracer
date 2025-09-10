#pragma once

#include "rayTracer.h"

namespace RayTracer {
    RayTracer::RayTracer()
    {
        m_spheres = {
            Sphere({ {0.0f, 0.0f, 3.0f}, 1.0f, {1.0f, 0.0f, 0.0f} }),
            Sphere({ {1.0f, 0.0f, 4.0f}, 0.5f, {0.5f, 1.0f, 0.0f} }),
            Sphere({ {0.0f, 2.5f, 0.0f}, 2.0f, {0.0f, 0.5f, 1.0f} })
        };
    }

    std::vector<glm::vec3> RayTracer::run(int bounceLimit, Renderer* renderer)
    {
        FrameBufferSettings frameBufferSize = renderer->getFrameBufferSize();
        std::vector<glm::vec3> frameBuffer(frameBufferSize.width * frameBufferSize.height);

        Camera camera;
        camera.fov = 45.0f;
        camera.location = glm::vec3(0.0f, 0.0f, -5.0f);

        float rayFactor = glm::tan(glm::radians(camera.fov) / 2.0f);
        float aspectRatio = frameBufferSize.width / static_cast<float>(frameBufferSize.height);

        for (int i = 0; i < frameBufferSize.height; i++) {
            for (int j = 0; j < frameBufferSize.width; j++) {
                Ray ray;
                ray.origin = camera.location;
                ray.direction = glm::normalize(glm::vec3(
                    (2.0f * (j + 0.5f) / frameBufferSize.width - 1.0f) * rayFactor * aspectRatio,
                    (1.0f - 2.0f * (i + 0.5f) / frameBufferSize.height) * rayFactor,
                    1.0f
                ));

                frameBuffer[i * frameBufferSize.width + j] = traceRay(ray, m_spheres);;
            }
        }

        return frameBuffer;
    }

    glm::vec3 RayTracer::traceRay(Ray& ray, const std::vector<Sphere>& spheres)
    {
        float closestIntersection = std::numeric_limits<float>::max();

        glm::vec3 hitColour(0.0f);

        for (auto sphere : spheres) {
            float intersection;
            if (isRayIntersectSphere(ray, sphere, intersection)) {
                if (intersection < closestIntersection) {
                    closestIntersection = intersection;
                    hitColour = sphere.colour;
                }
            }
        }

        if (closestIntersection < std::numeric_limits<float>::max()) {
            return hitColour;
        }

        return glm::vec3(0.0f);
    }

    bool RayTracer::isRayIntersectSphere(const Ray& ray, const Sphere& sphere, float& intersection)
    {
        // Assuming ray direction is normalised
        // Causes aTerm = 1
        float bTerm = glm::dot(ray.origin - sphere.centre, ray.direction);
        float cTerm = glm::dot(ray.origin - sphere.centre, ray.origin - sphere.centre) - sphere.radius * sphere.radius;

        float determinant = bTerm * bTerm - cTerm;

        if (determinant < 0) {
            return false;
        }

        float determinantSqrt = glm::sqrt(determinant);
        float term0 = -bTerm + determinantSqrt;
        float term1 = -bTerm - determinantSqrt;

        //term1 < term0
        intersection = (term1 > 0.001f) ? term1 : term0;
        return true;
    }
}

