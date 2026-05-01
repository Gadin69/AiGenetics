#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "ProjectionMatrix.h"

namespace Engine {
namespace Rendering {

// Custom projection modes for advanced rendering
class ProjectionModes {
public:
    // Orthographic projection with custom scaling
    static ProjectionMatrix OrthographicCustom(float width, float height, 
                                              float nearPlane, float farPlane, 
                                              float scaleX = 1.0f, float scaleY = 1.0f) {
        ProjectionMatrix proj(ProjectionMatrix::Type::ORTHOGRAPHIC);
        proj.SetOrthographic(width * scaleX, height * scaleY, nearPlane, farPlane);
        return proj;
    }
    
    // Perspective projection with adjustable field of view and aspect ratio
    static ProjectionMatrix PerspectiveCustom(float fov, float aspectRatio, 
                                             float nearPlane, float farPlane, 
                                             float focalLength = 1.0f) {
        ProjectionMatrix proj(ProjectionMatrix::Type::PERSPECTIVE);
        proj.SetPerspective(fov, aspectRatio, nearPlane, farPlane);
        return proj;
    }
    
    // Isometric projection matrix
    static ProjectionMatrix Isometric() {
        ProjectionMatrix proj(ProjectionMatrix::Type::CUSTOM);
        DirectX::XMFLOAT4X4 matrix;
        
        // Isometric projection matrix (30 degrees rotation)
        matrix._11 = 0.866f; matrix._12 = 0.0f;   matrix._13 = -0.5f;   matrix._14 = 0.0f;
        matrix._21 = 0.0f;   matrix._22 = 0.866f; matrix._23 = 0.0f;    matrix._24 = 0.0f;
        matrix._31 = 0.5f;   matrix._32 = 0.0f;   matrix._33 = 0.866f; matrix._34 = 0.0f;
        matrix._41 = 0.0f;   matrix._42 = 0.0f;   matrix._43 = 0.0f;    matrix._44 = 1.0f;
        
        proj.SetCustom(matrix);
        return proj;
    }
    
    // Oblique projection matrix
    static ProjectionMatrix Oblique(float angle = 45.0f) {
        ProjectionMatrix proj(ProjectionMatrix::Type::CUSTOM);
        DirectX::XMFLOAT4X4 matrix;
        
        // Oblique projection matrix
        float tanAngle = tanf(angle * DirectX::XM_PI / 180.0f);
        matrix._11 = 1.0f; matrix._12 = 0.0f; matrix._13 = 0.0f; matrix._14 = 0.0f;
        matrix._21 = 0.0f; matrix._22 = 1.0f; matrix._23 = 0.0f; matrix._24 = 0.0f;
        matrix._31 = tanAngle; matrix._32 = 0.0f; matrix._33 = 1.0f; matrix._34 = 0.0f;
        matrix._41 = 0.0f; matrix._42 = 0.0f; matrix._43 = 0.0f; matrix._44 = 1.0f;
        
        proj.SetCustom(matrix);
        return proj;
    }
    
    // Custom projection for VR/AR applications
    static ProjectionMatrix VRProjection(float eyeSeparation = 0.065f, 
                                       float focalDistance = 1.0f) {
        ProjectionMatrix proj(ProjectionMatrix::Type::CUSTOM);
        DirectX::XMFLOAT4X4 matrix;
        
        // Simple VR projection matrix
        matrix._11 = 1.0f; matrix._12 = 0.0f; matrix._13 = 0.0f; matrix._14 = 0.0f;
        matrix._21 = 0.0f; matrix._22 = 1.0f; matrix._23 = 0.0f; matrix._24 = 0.0f;
        matrix._31 = 0.0f; matrix._32 = 0.0f; matrix._33 = 1.0f; matrix._34 = 0.0f;
        matrix._41 = 0.0f; matrix._42 = 0.0f; matrix._43 = 0.0f; matrix._44 = 1.0f;
        
        proj.SetCustom(matrix);
        return proj;
    }
};

} // namespace Rendering
} // namespace Engine