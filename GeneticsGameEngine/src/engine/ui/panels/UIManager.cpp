#include "UIManager.h"
#include "../../../genetics/GeneticsIntegration.h"
#include <iostream>

// Undefine windows.h max/min macros if defined (from Skeleton.h include chain)
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace Engine {
namespace UI {

UIManager::UIManager() = default;
UIManager::~UIManager() = default;

void UIManager::RenderPanels(
    GeneticsIntegration* geneticsIntegration,
    Engine::Rendering::BaseCameraController* camera,
    int frameCount,
    bool& wireframeMode,
    GraphicsEngine* graphicsEngine
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
        RenderDebugPanel(geneticsIntegration, frameCount, graphicsEngine);
    }
    
    // Graphics Options Panel
    if (m_showGraphicsPanel)
    {
        RenderGraphicsPanel();
    }
    
    // Skeleton Debug Panel (Phase 7)
    if (m_showSkeletonPanel)
    {
        RenderSkeletonPanel(geneticsIntegration);
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
    ImGui::Checkbox("Skeleton Debug", &m_showSkeletonPanel);
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

void UIManager::RenderDebugPanel(GeneticsIntegration* geneticsIntegration, int frameCount, GraphicsEngine* graphicsEngine)
{
    ImGui::Begin("Debug Panel", &m_showDebugPanel);
    ImGui::Text("Performance:");
    ImGui::Text("  FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("  Frame: %d", frameCount);
    ImGui::Separator();
    
    ImGui::Text("Scene:");
    ImGui::Text("  Creatures: %zu", geneticsIntegration->GetCreatureMeshes().size());
    
    ImGui::Separator();
    ImGui::Text("Creature Generation:");
    
    // Seed input for reproducible creature generation
    static int seed = 0;
    static bool useRandomSeed = true;
    
    ImGui::Checkbox("Random Seed on Startup", &useRandomSeed);
    if (useRandomSeed)
    {
        ImGui::Text("  Current seed: Auto-generated (random)");
    }
    else
    {
        ImGui::InputInt("Seed", &seed);
        ImGui::Text("  Same seed = same creatures");
    }
    
    ImGui::Spacing();
    
    // Regenerate creatures button
    if (ImGui::Button("Regenerate Creatures (New Random)", ImVec2(-1, 0)))
    {
        if (graphicsEngine && graphicsEngine->GetDevice())
        {
            // Generate a new random seed
            seed = static_cast<int>(std::rand());
            useRandomSeed = false;
            
            std::cout << "[UI] Regenerating creatures with random seed: " << seed << std::endl;
            geneticsIntegration->RegenerateCreaturesWithSeed(
                static_cast<uint32_t>(seed),
                graphicsEngine->GetDevice(),
                graphicsEngine->GetCommandList()
            );
        }
    }
    
    if (ImGui::Button("Regenerate Creatures (Current Seed)", ImVec2(-1, 0)))
    {
        if (graphicsEngine && graphicsEngine->GetDevice())
        {
            std::cout << "[UI] Regenerating creatures with seed: " << seed << std::endl;
            geneticsIntegration->RegenerateCreaturesWithSeed(
                static_cast<uint32_t>(seed),
                graphicsEngine->GetDevice(),
                graphicsEngine->GetCommandList()
            );
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Mesh Generation Controls:");
    
    // Voxel size slider
    static float voxelSize = 0.031f; // User-tuned default
    ImGui::SliderFloat("Voxel Size", &voxelSize, 0.01f, 0.10f, "%.3f");
    ImGui::Text("  Smaller = higher detail, slower");
    
    // Falloff multiplier slider
    static float falloffMultiplier = 1.2f; // User-tuned default
    ImGui::SliderFloat("Falloff Radius", &falloffMultiplier, 0.5f, 5.0f, "%.1fx");
    ImGui::Text("  Higher = more blobby, Lower = more defined");
    
    ImGui::Spacing();
    
    // Regenerate button
    if (ImGui::Button("Regenerate Meshes", ImVec2(-1, 0)))
    {
        // Set falloff on scalar field generator
        geneticsIntegration->GetScalarFieldGenerator().SetFalloffMultiplier(falloffMultiplier);
        // Regenerate all meshes
        geneticsIntegration->RegenerateMeshes(voxelSize, falloffMultiplier);
    }
    
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

// Phase 7: Skeleton Debug Panel
void UIManager::RenderSkeletonPanel(GeneticsIntegration* geneticsIntegration)
{
    ImGui::Begin("Skeleton Debug Panel", &m_showSkeletonPanel, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Phase 7: Skeletal Animation System");
    ImGui::Separator();
    
    const auto& creatureMeshes = geneticsIntegration->GetCreatureMeshes();
    
    if (creatureMeshes.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No creatures loaded");
        ImGui::End();
        return;
    }
    
    // Creature selector
    static int selectedCreature = 0;
    
    // Rebuild combo with all creature names
    std::vector<const char*> creatureNames;
    for (const auto& mesh : creatureMeshes)
    {
        creatureNames.push_back(mesh.creatureID.c_str());
    }
    
    if (ImGui::Combo("Creature", &selectedCreature, 
                     creatureNames.data(), creatureNames.size()))
    {
        // Selection changed
    }
    
    if (selectedCreature < 0 || selectedCreature >= (int)creatureMeshes.size())
    {
        selectedCreature = 0;
    }
    
    const auto& creature = creatureMeshes[selectedCreature];
    
    ImGui::Separator();
    ImGui::Text("Creature: %s", creature.creatureID.c_str());
    ImGui::Text("Mesh: %u vertices, %u triangles", 
                creature.meshRenderer->GetVertexCount(),
                creature.mesh.indices.size() / 3);
    
    ImGui::Separator();
    
    // Display skeleton info
    if (creature.skeleton)
    {
        const auto& skeleton = *creature.skeleton;
        const auto& bones = skeleton.GetBones();
        
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Skeleton Generated");
        ImGui::Text("Total Bones: %zu", bones.size());
        
        // Skeleton visualization toggle
        ImGui::Spacing();
        // Need to access via non-const reference from the vector
        bool& showVis = const_cast<bool&>(creature.showSkeletonVisualization);
        if (ImGui::Checkbox("Show Skeleton Visualization", &showVis))
        {
            // Toggle changed
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(Yellow wireframe lines)");
        
        ImGui::Spacing();
        ImGui::Text("Bone Hierarchy:");
        
        // Show first 20 bones to avoid UI clutter
        int boneCount = std::min((int)bones.size(), 20);
        for (int i = 0; i < boneCount; ++i)
        {
            const auto& bone = bones[i];
            
            // Indent based on hierarchy depth
            int depth = 0;
            int parentIdx = bone.parentIndex;
            while (parentIdx != -1 && parentIdx < (int)bones.size())
            {
                depth++;
                parentIdx = bones[parentIdx].parentIndex;
                if (depth > 10) break; // Safety limit
            }
            
            ImGui::Indent(depth * 16.0f);
            
            // Bone name and parent info
            if (bone.parentIndex == -1)
            {
                ImGui::Text("[%d] %s (ROOT)", i, bone.name.c_str());
            }
            else
            {
                ImGui::Text("[%d] %s (parent: %d)", i, bone.name.c_str(), bone.parentIndex);
            }
            
            // Bone dimensions
            ImGui::Indent(16.0f);
            ImGui::Text("Length: (%.2f, %.2f, %.2f)", 
                       bone.boneLength.x, bone.boneLength.y, bone.boneLength.z);
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", 
                       bone.localPosition.x, bone.localPosition.y, bone.localPosition.z);
            ImGui::Unindent(16.0f);
            
            ImGui::Unindent(depth * 16.0f);
        }
        
        if ((int)bones.size() > 20)
        {
            ImGui::Text("... and %zu more bones", bones.size() - 20);
        }
        
        ImGui::Separator();
        
        // Skeleton statistics
        ImGui::Text("Skeleton Statistics:");
        int rootBones = 0;
        float totalMass = 0.0f;
        int limbBones = 0;
        int organBones = 0;
        
        for (const auto& bone : bones)
        {
            if (bone.parentIndex == -1) rootBones++;
            totalMass += bone.mass;
            
            // Classify bone type by name
            if (bone.name.find("Leg") != std::string::npos ||
                bone.name.find("Arm") != std::string::npos ||
                bone.name.find("Wing") != std::string::npos ||
                bone.name.find("Tentacle") != std::string::npos)
            {
                limbBones++;
            }
            else if (bone.name.find("Eye") != std::string::npos ||
                     bone.name.find("Organ") != std::string::npos)
            {
                organBones++;
            }
        }
        
        ImGui::BulletText("Root bones: %d", rootBones);
        ImGui::BulletText("Limb bones: %d", limbBones);
        ImGui::BulletText("Organ bones: %d", organBones);
        ImGui::BulletText("Total mass: %.2f", totalMass);
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Info:");
        ImGui::BulletText("Bones act as metaball attractors");
        ImGui::BulletText("Mesh wraps around skeleton");
        ImGui::BulletText("Genetics control bone placement");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "✗ No skeleton available");
        ImGui::Text("Skeleton generation failed or not implemented");
    }
    
    ImGui::End();
}

} // namespace UI
} // namespace Engine
