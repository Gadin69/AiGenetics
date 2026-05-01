#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include "CameraTypes.h"
#include "CameraController.h"
#include "OrbitCameraController.h"
#include "FirstPersonCameraController.h"
#include "CinematicCameraController.h"

namespace Engine {
namespace Rendering {

// Camera system that manages multiple camera controllers
class CameraSystem {
private:
    std::vector<std::unique_ptr<BaseCameraController>> m_cameras;
    std::unordered_map<std::string, std::unique_ptr<BaseCameraController>> m_namedCameras;
    
    // Active camera
    BaseCameraController* m_activeCamera;
    
public:
    CameraSystem() : m_activeCamera(nullptr) {}
    
    // Create and add a new camera
    template<typename T>
    T* CreateCamera(const std::string& name = "") {
        auto camera = std::make_unique<T>();
        T* ptr = camera.get();
        
        if (!name.empty()) {
            m_namedCameras[name] = std::move(camera);
        } else {
            m_cameras.push_back(std::move(camera));
        }
        
        return ptr;
    }
    
    // Get camera by name
    BaseCameraController* GetCamera(const std::string& name) {
        auto it = m_namedCameras.find(name);
        if (it != m_namedCameras.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    // Set active camera
    void SetActiveCamera(BaseCameraController* camera) {
        m_activeCamera = camera;
    }
    
    // Get active camera
    BaseCameraController* GetActiveCamera() const {
        return m_activeCamera;
    }
    
    // Update all cameras
    void Update(float deltaTime) {
        for (auto& camera : m_cameras) {
            camera->Update(deltaTime);
        }
        
        for (auto& pair : m_namedCameras) {
            pair.second->Update(deltaTime);
        }
    }
};

} // namespace Rendering
} // namespace Engine