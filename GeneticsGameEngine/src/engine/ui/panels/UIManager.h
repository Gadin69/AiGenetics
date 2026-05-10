#pragma once

#include <cstddef>
#include <functional>
#include "../../../third_party/imgui/imgui.h"
#include "../../genetics/GeneticsIntegration.h"
#include "../rendering/camera/CameraTypes.h"
#include "../../animation/Skeleton.h"

// Forward declaration
class GraphicsEngine;

namespace Engine {
namespace UI {

// UIManager handles all ImGui panel rendering and state management
class UIManager {
public:
    UIManager();
    ~UIManager();

    // Render all UI panels
    void RenderPanels(
        GeneticsIntegration* geneticsIntegration,
        Engine::Rendering::BaseCameraController* camera,
        int frameCount,
        bool& wireframeMode,
        GraphicsEngine* graphicsEngine = nullptr
    );

    // Set quit callback
    void SetQuitCallback(std::function<void()> callback) { m_quitCallback = callback; }
    
    // Set fullscreen callback
    void SetFullscreenCallback(std::function<void()> callback) { m_fullscreenCallback = callback; }

    // Panel visibility getters/setters
    bool IsControlPanelVisible() const { return m_showControlPanel; }
    void SetControlPanelVisible(bool visible) { m_showControlPanel = visible; }
    
    bool IsDemoWindowVisible() const { return m_showDemoWindow; }
    void SetDemoWindowVisible(bool visible) { m_showDemoWindow = visible; }
    
    bool IsCreaturePanelVisible() const { return m_showCreaturePanel; }
    void SetCreaturePanelVisible(bool visible) { m_showCreaturePanel = visible; }
    
    bool IsDebugPanelVisible() const { return m_showDebugPanel; }
    void SetDebugPanelVisible(bool visible) { m_showDebugPanel = visible; }
    
    bool IsGraphicsPanelVisible() const { return m_showGraphicsPanel; }
    void SetGraphicsPanelVisible(bool visible) { m_showGraphicsPanel = visible; }
    
    bool IsSkeletonPanelVisible() const { return m_showSkeletonPanel; }
    void SetSkeletonPanelVisible(bool visible) { m_showSkeletonPanel = visible; }

private:
    // Panel visibility toggles
    bool m_showControlPanel = true;
    bool m_showDemoWindow = false;
    bool m_showCreaturePanel = false;
    bool m_showDebugPanel = false;
    bool m_showGraphicsPanel = false;
    bool m_showSkeletonPanel = false;
    
    // Quit callback
    std::function<void()> m_quitCallback;
    
    // Fullscreen callback
    std::function<void()> m_fullscreenCallback;

    // Individual panel renderers
    void RenderControlPanel(bool& wireframeMode);
    void RenderCreaturePanel(GeneticsIntegration* geneticsIntegration);
    void RenderDebugPanel(GeneticsIntegration* geneticsIntegration, int frameCount, GraphicsEngine* graphicsEngine);
    void RenderGraphicsPanel();
    void RenderSkeletonPanel(GeneticsIntegration* geneticsIntegration);
};

} // namespace UI
} // namespace Engine
