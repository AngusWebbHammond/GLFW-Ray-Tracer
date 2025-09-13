#pragma once

#include "ui.h"
#include <glm/gtc/type_ptr.hpp>

namespace RayTracer::UI {
    namespace {
        void createImGuiDockspace()
        {
            static bool isFullScreen = true;
            static ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None;

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;

            if (isFullScreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

            ImGui::Begin("DockSpace", nullptr, windowFlags);
            ImGui::PopStyleVar(2);

            // DockSpace
            ImGuiID dockSpaceID = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockSpaceID, ImVec2(0.0f, 0.0f), dockSpaceFlags);
            ImGui::End();
        }

        void createImGuiViewport(Renderer* renderer)
        {
            if (ImGui::Begin("Viewport")) {
                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
                renderer->setWidthAndHeight(viewportPanelSize.x, viewportPanelSize.y);
                ImGui::Image((void*)(intptr_t)renderer->getTexture(), viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::End();
            }
        }
    }

    void createImGuiPropertiesPanel(RayTracer& rayTracer)
    {
        if (ImGui::Begin("Properties")) {
            for (Sphere& sphere : rayTracer.m_spheres) {
                ImGui::PushID(&sphere);

                ImGui::DragFloat3("Centre", glm::value_ptr(sphere.centre), 0.1f);
                ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
                ImGui::ColorEdit3("Sphere Colour", glm::value_ptr(sphere.material.materialColour));

                ImGui::DragFloat("Reflectivness", &sphere.material.reflectivness, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Emission Strength", &sphere.material.emissiveStrength, 0.1f, 0.0f);
                ImGui::ColorEdit3("Emission Colour", glm::value_ptr(sphere.material.emissionColour));

                ImGui::PopID();
                ImGui::Separator();
            }

            ImGui::ColorEdit3("Background Colour", glm::value_ptr(rayTracer.m_background));
            ImGui::End();
        }
    }

    void initImGui(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450");
    }

    void cleanupImGui() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void createImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void createImGuiWindows(Renderer* renderer)
    {
        createImGuiDockspace();
        createImGuiViewport(renderer);
    }

    void renderImGui()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
