#include "UIManager.h"
#include "../../../genetics/GeneticsIntegration.h"

namespace Engine {
namespace UI {

UIManager::UIManager() = default;
UIManager::~UIManager() = default;

void UIManager::RenderPanels(
    GeneticsIntegration* geneticsIntegration,
    Engine::Rendering::BaseCameraController* camera,
    int frameCount,
    bool& wireframeMode
) {
    // Main Control Panel (always visible)
    if (m_showControlPanel)
    {
        RenderControlPanel(wireframeMode);
    }
    
    // ImGui Demo Window (toggleable)
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
    
    // Creature Selector Panel
    if (m_showCreaturePanel)
    {
        RenderCreaturePanel(geneticsIntegration);
    }
    
    // Debug Panel
    if (m_showDebugPanel)
    {
        RenderDebugPanel(geneticsIntegration, frameCount);
    }
}

void UIManager::RenderControlPanel(bool& wireframeMode)
{
    ImGui::Begin("Main Control Panel", &m_showControlPanel, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Genetics Game Engine");
    ImGui::Separator();
    
    ImGui::Text("Panel Toggles:");
    ImGui::Checkbox("Demo Window", &m_showDemoWindow);
    ImGui::Checkbox("Creature Selector", &m_showCreaturePanel);
    ImGui::Checkbox("Debug Panel", &m_showDebugPanel);
    ImGui::Spacing();
    
    ImGui::Text("Rendering:");
    ImGui::Checkbox("Wireframe Mode", &wireframeMode);
    
    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::BulletText("WASD - Move camera");
    ImGui::BulletText("Mouse - Look around");
    ImGui::BulletText("Escape - Toggle this panel");
    
    ImGui::Separator();
    
    // Quit button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Quit Application", ImVec2(-1, 0)))
    {
        if (m_quitCallback)
        {
            m_quitCallback();
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::End();
}

void UIManager::RenderCreaturePanel(GeneticsIntegration* geneticsIntegration)
{
    ImGui::Begin("Creature Selector", &m_showCreaturePanel);
    ImGui::Text("Active Creatures:");
    ImGui::Separator();
    
    const auto& creatureMeshes = geneticsIntegration->GetCreatureMeshes();
    for (size_t i = 0; i < creatureMeshes.size(); ++i)
    {
        ImGui::Text("Creature %zu: %s", i, creatureMeshes[i].creatureID.c_str());
        ImGui::Text("  Vertices: %u", creatureMeshes[i].meshRenderer->GetVertexCount());
        ImGui::Spacing();
    }
    
    ImGui::End();
}

void UIManager::RenderDebugPanel(GeneticsIntegration* geneticsIntegration, int frameCount)
{
    ImGui::Begin("Debug Panel", &m_showDebugPanel);
    ImGui::Text("Performance:");
    ImGui::Text("  FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("  Frame: %d", frameCount);
    ImGui::Separator();
    
    ImGui::Text("Scene:");
    ImGui::Text("  Creatures: %zu", geneticsIntegration->GetCreatureMeshes().size());
    
    ImGui::End();
}

} // namespace UI
} // namespace Engine
