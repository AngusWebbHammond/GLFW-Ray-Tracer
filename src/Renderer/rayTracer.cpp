#pragma once

#include "rayTracer.h"

namespace RayTracer {
    RayTracer::RayTracer()
    {
        Material material1 = Material({ 1.0f, 0.0f, 0.0f });
        Material material2 = Material({ 0.0f, 1.0f, 0.0f });
        Material material3 = Material({ 0.0f, 0.0f, 1.0f });

        material1.emissionColour = glm::vec3(1.0f);
        material1.emissiveStrength = 1.0f;
        material2.reflectivness = 1.0f;

        m_spheres = {
            Sphere({ {1.0f, 0.0f, 3.0f}, 1.0f, material1 }),
            Sphere({ {-1.0f, 0.0f, 3.0f}, 1.0f, material2 }),
            Sphere({ {0.0f, 2.0f, 3.0f}, 1.0f, material3 })
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

                glm::vec3 colour(0.0f);
                glm::vec3 attenuation(1.0f);
                bool canBounce = true;

                for (int t = 0; t < bounceLimit; t++) {
                    glm::vec3 bounceColour = traceRay(ray, m_spheres, canBounce);
                    colour += attenuation * bounceColour;
                    attenuation *= 0.5f;
                    if (!canBounce) break;
                }

                frameBuffer[i * frameBufferSize.width + j] = colour;
            }
        }

        return frameBuffer;
    }

    glm::vec3 RayTracer::traceRay(Ray& ray, const std::vector<Sphere>& spheres, bool& canBounce)
    {

        float closestIntersection = std::numeric_limits<float>::max();

        HitSphere hitSphere = HitSphere({ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), Material({{ 0.0f, 0.0f, 0.0f }}) });

        for (auto sphere : spheres) {
            float intersection;
            if (isRayIntersectSphere(ray, sphere, intersection)) {
                if (intersection < closestIntersection) {
                    if (sphere.material.reflectivness < 0.001f) {
                        canBounce = false;
                    }
                    else {
                        canBounce = true;
                    }

                    closestIntersection = intersection;
                    hitSphere.hitColour = sphere.material.materialColour;
                    hitSphere.hitPoint = ray.origin + intersection * ray.direction;
                    hitSphere.hitNormal = glm::normalize(hitSphere.hitPoint - sphere.centre);
                    hitSphere.hitMaterial = sphere.material;
                }
            }
        }

        if (closestIntersection < std::numeric_limits<float>::max()) {
            glm::vec3 colour = hitSphere.hitMaterial.emissionColour * hitSphere.hitMaterial.emissiveStrength;

            ray.origin = hitSphere.hitPoint + 0.001f * hitSphere.hitNormal;
            ray.direction = glm::reflect(ray.direction, hitSphere.hitNormal);

            return colour + hitSphere.hitColour * hitSphere.hitMaterial.reflectivness;
        }

        return glm::vec3(0.5f, 0.0f, 0.2f);
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

    Material::Material(glm::vec3 colour)
    {
        materialColour = colour;
        reflectivness = 0.0f;
        metallicness = 0.0f;

        emissiveStrength = 0.0f;
        emissionColour = glm::vec3(0.0f);
    }
}

