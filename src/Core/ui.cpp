#pragma once

#include "ui.h"

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
                ImGui::Text("Test");
                ImGui::End();
            }
        }

        void createImGuiPropertiesPanel()
        {
            if (ImGui::Begin("Properties")) {
                ImGui::End();
            }
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
        createImGuiPropertiesPanel();
    }

    void renderImGui()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
