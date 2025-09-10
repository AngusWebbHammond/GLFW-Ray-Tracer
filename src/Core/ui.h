#pragma once

#include <glad/gl.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "../Renderer/renderer.h"

namespace RayTracer::UI {
    namespace {
        void createImGuiDockspace();
        void createImGuiViewport(Renderer* renderer);
        void createImGuiPropertiesPanel();
    }

    void initImGui(GLFWwindow* window);
    void cleanupImGui();
    void createImGuiFrame();
    void createImGuiWindows(Renderer* renderer);

    void renderImGui();
}