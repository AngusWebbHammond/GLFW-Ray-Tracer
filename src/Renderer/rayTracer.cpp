#pragma once

#include <iostream>

#include "rayTracer.h"

namespace RayTracer {
    RayTracer::RayTracer()
    {
        Material material1 = Material({ 1.0f, 1.0f, 1.0f });
        Material material2 = Material({ 1.0f, 1.0f, 1.0f });
        Material material3 = Material({ 0.0f, 0.4f, 1.0f });
        // Material material4 = Material({ 1.0f, 1.0f, 1.0f });

        // material4.emissionColour = glm::vec3(1.0f);
        // material4.emissiveStrength = 1.0f;

        material1.emissionColour = glm::vec3(1.0f);
        material1.emissiveStrength = 10.0f;
        material2.reflectivness = 1.0f;
        material3.reflectivness = 1.0f;

        m_spheres = {
            Sphere({ {2.9f, -0.4f, -0.2f}, 1.0f, material1 }),
            Sphere({ {0.0f, 0.2f, 0.0f}, 1.0f, material2 }),
            Sphere({ {0.0f, 1.0f, -1.0f}, 0.5f, material2 }),
            Sphere({ {0.0f, 19.7f, 0.0f}, 18.3f, material3 }),
            // Sphere({ {2.9f, -0.4f, -3.0f}, 1.0f, material4 }),
        };

        m_random = std::mt19937();
        m_accumilate = false;
        m_frames = 1;
        m_background = glm::vec3(0.5f);
    }

    std::vector<glm::vec3> RayTracer::run(int bounceLimit, Renderer* renderer)
    {
        FrameBufferSettings frameBufferSize = renderer->getFrameBufferSize();
        std::vector<glm::vec3> frameBuffer(frameBufferSize.width * frameBufferSize.height);

        if (m_accumilateFrameBuffer.empty() || frameBuffer.size() != m_accumilateFrameBuffer.size()) {
            m_accumilateFrameBuffer.resize(frameBufferSize.width * frameBufferSize.height, glm::vec3(0.0f));
        }

        Camera camera;
        camera.fov = 45.0f;
        camera.location = glm::vec3(0.0f, 0.0f, -10.0f);

        float rayFactor = glm::tan(glm::radians(camera.fov) / 2.0f);
        float aspectRatio = frameBufferSize.width / static_cast<float>(frameBufferSize.height);

        if (m_accumilate) m_frames++;
        else {
            m_frames = 1;
        }

        for (int i = 0; i < frameBufferSize.height; i++) {
            for (int j = 0; j < frameBufferSize.width; j++) {
                Ray ray;
                ray.origin = camera.location;
                ray.direction = glm::normalize(glm::vec3(
                    (2.0f * (j + 0.5f) / frameBufferSize.width - 1.0f) * rayFactor * aspectRatio,
                    (1.0f - 2.0f * (i + 0.5f) / frameBufferSize.height) * rayFactor,
                    1.0f
                ));

                glm::vec3 colour = traceRay(ray, m_spheres, bounceLimit);

                if (m_accumilate) {
                    m_accumilateFrameBuffer[i * frameBufferSize.width + j] += colour;
                    frameBuffer[i * frameBufferSize.width + j] = m_accumilateFrameBuffer[i * frameBufferSize.width + j] / glm::vec3(m_frames);
                }

                else {
                    m_accumilateFrameBuffer[i * frameBufferSize.width + j] = glm::vec3(0.0f);
                    frameBuffer[i * frameBufferSize.width + j] = colour;
                }
            }
        }

        return frameBuffer;
    }

    glm::vec3 RayTracer::traceRay(Ray& ray, const std::vector<Sphere>& spheres, int bounceLimit)
    {
        glm::vec3 colour(0.0f);
        glm::vec3 attenuation(1.0f);

        HitSphere hitSphere = HitSphere({ glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material({{ 0.0f, 0.0f, 0.0f }}), glm::vec3(0.0f) });

        for (int t = 0; t < bounceLimit; t++) {

            glm::vec3 bounceColour{ 0.0f };

            float closestIntersection = std::numeric_limits<float>::max();
            for (auto sphere : spheres) {
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
                float randomFloat = fabs(m_uniformDistribution(m_random)) / m_uniformDistribution.max();

                if (randomFloat < hitSphere.hitMaterial.reflectivness) {
                    // Specular Bounce
                    ray.direction = glm::reflect(ray.direction, hitSphere.hitNormal);
                }
                else {
                    // Diffuse Bounce
                    glm::vec3 randomNum = glm::normalize(getRandomOnUnitSphere());
                    if (glm::dot(randomNum, hitSphere.hitNormal) < 0) {
                        randomNum = glm::normalize(glm::reflect(randomNum, hitSphere.hitNormal));
                    }
                    ray.direction = randomNum;
                }

                colour += hitSphere.hitLight * hitSphere.hitColour;

                // Hit light, stop bouncing
                // if (glm::dot(hitSphere.hitLight, hitSphere.hitLight) > 0.001f) {
                //     return colour;
                // }

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

    glm::vec3 RayTracer::getRandomOnUnitSphere()
    {
        // Generates a point on a unit sphere, with uniform distribution
        // https://corysimon.github.io/articles/uniformdistn-on-sphere/
        double theta = 2 * 3.1415927 * m_uniformDistribution(m_random);
        double phi = acos(1 - 2 * m_uniformDistribution(m_random));
        double x = sin(phi) * cos(theta);
        double y = sin(phi) * sin(theta);
        double z = cos(phi);
        return glm::vec3(x, y, z);
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

