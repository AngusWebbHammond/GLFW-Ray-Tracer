#pragma once

#include <iostream>

#include "rayTracer.h"

namespace RayTracer {
    RayTracer::RayTracer() : m_randomNumber(123456789)
    {
        Material material1 = Material({ 1.0f, 1.0f, 1.0f });
        Material material2 = Material({ 1.0f, 1.0f, 1.0f });
        Material material3 = Material({ 1.0f, 0.9f, 0.4f });
        Material material4 = Material({ 1.0f, 0.3f, 0.0f });
        Material material5 = Material({ 0.5f, 1.0f, 1.0f });

        material1.emissionColour = glm::vec3(1.0f);
        material1.emissiveStrength = 20.0f;

        material5.reflectivness = 0.65f;

        m_spheres = {
            Sphere({ {0.0f, -10.0f, -6.0f}, 3.0f, material1 }),
            Sphere({ {0.0f, 0.0f, -2.5f}, 1.0f, material2 }),
            Sphere({ {3.0f, 0.0f, -3.0f}, 1.25f, material3 }),
            Sphere({ {-3.0f, 0.0f, -3.0f}, 1.25f, material4 }),
            Sphere({ {0.0f, 20.0f, -3.0f}, 19.0f, material5 }),
        };

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
        int fbHeight = frameBufferSize.height;
        int fbWidth = frameBufferSize.width;

        float rayFactor = glm::tan(glm::radians(camera.fov) / 2.0f);
        float aspectRatio = frameBufferSize.width / static_cast<float>(frameBufferSize.height);

        float rayFactorAR = rayFactor * aspectRatio;

        if (m_accumilate) m_frames++;
        else {
            m_frames = 1;
        }

        for (int i = 0; i < fbHeight; i++) {
            for (int j = 0; j < fbWidth; j++) {
                Ray ray;
                ray.origin = camera.location;
                ray.direction = glm::normalize(glm::vec3(
                    (2.0f * (j + 0.5f) / frameBufferSize.width - 1.0f) * rayFactorAR,
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
            for (auto &sphere : spheres) {
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
                ray.direction = glm::normalize((1 - hitSphere.hitMaterial.reflectivness) * (hitSphere.hitNormal + randomNum) + hitSphere.hitMaterial.reflectivness * glm::reflect(ray.direction, hitSphere.hitNormal));

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
        // Thanks to https://www.youtube.com/watch?v=Qz0KTGYJtUk&t=545s
        float x = m_normalDistribution(m_randomNumber);
        float y = m_normalDistribution(m_randomNumber);
        float z = m_normalDistribution(m_randomNumber);

		return glm::normalize(glm::vec3(x, y, z));
    }

    Random::Random(std::uint32_t seed)
    {
        m_randomNumber = seed;
    }

    std::uint32_t Random::getRandomFloat()
    {
        // Edit on an xor random number generator from 
        uint32_t result = m_randomNumber;
        result ^= result << 13;
        result ^= result >> 17;
        result ^= result << 5;
        m_randomNumber = result;

        return result;
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

