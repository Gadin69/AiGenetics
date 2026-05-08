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
    
    // Graphics Options Panel
    if (m_showGraphicsPanel)
    {
        RenderGraphicsPanel();
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
    ImGui::Checkbox("Graphics Options", &m_showGraphicsPanel);
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

void UIManager::RenderGraphicsPanel()
{
    ImGui::Begin("Graphics Options", &m_showGraphicsPanel, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Display Settings");
    ImGui::Separator();
    
    // Resolution selection
    static const char* resolutions[] = {
        "1920x1080 (Full HD)",
        "2560x1440 (QHD)",
        "3840x2160 (4K UHD)",
        "1280x720 (HD)",
        "1600x900 (HD+)"
    };
    static int currentResolution = 0;
    
    ImGui::Text("Resolution:");
    if (ImGui::BeginCombo("##Resolution", resolutions[currentResolution]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(resolutions); i++)
        {
            bool isSelected = (currentResolution == i);
            if (ImGui::Selectable(resolutions[i], isSelected))
            {
                currentResolution = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::Spacing();
    
    // Fullscreen toggle
    bool fullscreen = false; // Will be set by callback if needed
    if (ImGui::Checkbox("Fullscreen", &fullscreen))
    {
        if (m_fullscreenCallback)
        {
            m_fullscreenCallback();
        }
    }
    
    // VSync toggle
    static bool vsync = true;
    ImGui::Checkbox("VSync", &vsync);
    
    ImGui::Separator();
    ImGui::Text("Quality Settings");
    ImGui::Separator();
    
    // Quality preset
    static const char* qualityPresets[] = {
        "Low",
        "Medium",
        "High",
        "Ultra",
        "Custom"
    };
    static int currentQuality = 2; // Default to High
    
    ImGui::Text("Quality Preset:");
    if (ImGui::BeginCombo("##Quality", qualityPresets[currentQuality]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(qualityPresets); i++)
        {
            bool isSelected = (currentQuality == i);
            if (ImGui::Selectable(qualityPresets[i], isSelected))
            {
                currentQuality = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::Spacing();
    
    // Shadow quality
    static const char* shadowQualities[] = {"Off", "Low", "Medium", "High", "Ultra"};
    static int shadowQuality = 3;
    ImGui::Text("Shadow Quality:");
    ImGui::Combo("##Shadow", &shadowQuality, shadowQualities, IM_ARRAYSIZE(shadowQualities));
    
    // Anti-aliasing
    static const char* msaaOptions[] = {"Off", "2x MSAA", "4x MSAA", "8x MSAA"};
    static int msaaLevel = 2;
    ImGui::Text("Anti-Aliasing:");
    ImGui::Combo("##MSAA", &msaaLevel, msaaOptions, IM_ARRAYSIZE(msaaOptions));
    
    ImGui::Spacing();
    
    // Advanced rendering options
    ImGui::Text("Rendering Features:");
    static bool hdrEnabled = true;
    ImGui::Checkbox("HDR", &hdrEnabled);
    
    static bool bloomEnabled = true;
    ImGui::Checkbox("Bloom", &bloomEnabled);
    
    static bool ambientOcclusion = true;
    ImGui::Checkbox("Screen Space AO", &ambientOcclusion);
    
    static bool motionBlur = false;
    ImGui::Checkbox("Motion Blur", &motionBlur);
    
    static bool depthOfField = false;
    ImGui::Checkbox("Depth of Field", &depthOfField);
    
    ImGui::Separator();
    
    // Apply button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
    if (ImGui::Button("Apply Settings", ImVec2(-1, 0)))
    {
        // Settings would be applied here
        // TODO: Send settings to GraphicsEngine
    }
    ImGui::PopStyleColor(3);
    
    ImGui::Spacing();
    
    // Reset button
    if (ImGui::Button("Reset to Default", ImVec2(-1, 0)))
    {
        currentResolution = 0;
        fullscreen = false;
        vsync = true;
        currentQuality = 2;
        shadowQuality = 3;
        msaaLevel = 2;
        hdrEnabled = true;
        bloomEnabled = true;
        ambientOcclusion = true;
        motionBlur = false;
        depthOfField = false;
    }
    
    ImGui::End();
}

} // namespace UI
} // namespace Engine
