#include "MolluscaSkeletonGenerator.h"
#include <cmath>

using namespace DirectX;

namespace Engine {
namespace Animation {

Skeleton MolluscaSkeletonGenerator::GenerateSkeleton(
    const Engine::Genetics::Genome& genome,
    const Engine::Procedural::Generation::CreatureParams& params)
{
    Skeleton skeleton;
    
    // Genetic parameters for mollusk body plan
    float footLength = 1.5f * params.scaleFactor;
    float visceralMassSize = 0.6f * params.scaleFactor;
    int tentacleCount = params.limbCount;
    float tentacleLength = 0.8f * params.scaleFactor;
    
    // Check for shell presence (gene-driven)
    int shellGene = static_cast<int>(genome.GetGeneValue(0x1300) * 100.0f);
    bool hasShell = (shellGene > 30); // 70% chance of having a shell
    int shellTurns = 3 + (static_cast<int>(genome.GetGeneValue(0x1301) * 100.0f) % 5); // 3-7 spiral turns
    
    // STEP 1: Generate foot (locomotion organ)
    GenerateFoot(skeleton, footLength);
    
    // STEP 2: Generate visceral mass (main body)
    GenerateVisceralMass(skeleton, visceralMassSize);
    
    // STEP 3: Generate shell (if present)
    if (hasShell)
    {
        GenerateShell(skeleton, shellTurns, visceralMassSize * 1.5f);
    }
    
    // STEP 4: Generate tentacles (gene-driven count)
    int tentaclePairs = tentacleCount / 2;
    for (int i = 0; i < tentaclePairs; ++i)
    {
        GenerateTentaclePair(skeleton, 1, tentacleLength, i); // Attach to visceral mass
    }
    
    // Compute final world transforms
    skeleton.ComputeWorldTransforms();
    
    return skeleton;
}

void MolluscaSkeletonGenerator::GenerateFoot(Skeleton& skeleton, float footLength)
{
    XMFLOAT3 position = {-0.3f, 0.0f, 0.0f}; // Anterior end (negative X)
    XMFLOAT3 length = {0.25f, footLength * 0.7f, footLength}; // Y is foot width, Z is foot length
    
    skeleton.AddBone(CreateBone("Foot", position, length, -1), -1);
}

void MolluscaSkeletonGenerator::GenerateVisceralMass(Skeleton& skeleton, float massSize)
{
    XMFLOAT3 position = {0.3f, 0.0f, 0.0f}; // Posterior to foot (positive X)
    XMFLOAT3 length = {massSize * 1.0f, massSize * 1.2f, massSize * 1.2f}; // Larger visceral mass
    
    skeleton.AddBone(CreateBone("VisceralMass", position, length, 0), 0);
}

void MolluscaSkeletonGenerator::GenerateShell(Skeleton& skeleton, int spiralTurns, float shellSize)
{
    // Shell is a spiral structure built from multiple bone segments
    int parentIndex = 1; // Attach to visceral mass
    
    for (int i = 0; i < spiralTurns; ++i)
    {
        std::string name = "Shell_Turn_" + std::to_string(i);
        
        // Spiral positioning - relative to parent (spiral in X-Y plane, growing upward)
        float angle = i * XM_PIDIV2; // 90 degrees per turn
        float radius = shellSize * 0.3f;
        XMFLOAT3 position = {
            std::cos(angle) * radius,
            shellSize * 0.2f, // Growing upward in Y
            std::sin(angle) * radius
        };
        
        XMFLOAT3 length = {shellSize * 0.4f, shellSize * 0.3f, shellSize * 0.4f};
        
        skeleton.AddBone(CreateBone(name, position, length, parentIndex), parentIndex);
        parentIndex = static_cast<int>(skeleton.GetBoneCount()) - 1;
    }
}

void MolluscaSkeletonGenerator::GenerateTentaclePair(
    Skeleton& skeleton, int attachmentIndex, float tentacleLength, int tentacleIndex)
{
    // Tentacles are flexible, segmented structures
    int segments = 3;
    float segmentLength = tentacleLength / segments;
    
    // Left tentacle
    {
        std::string baseName = "Tentacle_Left_" + std::to_string(tentacleIndex);
        int parentIdx = attachmentIndex;
        
        for (int i = 0; i < segments; ++i)
        {
            std::string name = baseName + "_Seg_" + std::to_string(i);
            // Relative offset from parent (tentacles extend forward in -X direction)
            XMFLOAT3 position = { (i == 0) ? -0.3f : -segmentLength * 0.3f, 0.0f, (i == 0) ? 0.0f : 0.05f };
            XMFLOAT3 length = {segmentLength, 0.04f, 0.04f};
            
            skeleton.AddBone(CreateBone(name, position, length, parentIdx), parentIdx);
            parentIdx = static_cast<int>(skeleton.GetBoneCount()) - 1;
        }
    }
    
    // Right tentacle
    {
        std::string baseName = "Tentacle_Right_" + std::to_string(tentacleIndex);
        int parentIdx = attachmentIndex;
        
        for (int i = 0; i < segments; ++i)
        {
            std::string name = baseName + "_Seg_" + std::to_string(i);
            // Relative offset from parent (tentacles extend forward in -X direction)
            XMFLOAT3 position = { (i == 0) ? 0.3f : segmentLength * 0.3f, 0.0f, (i == 0) ? 0.0f : -0.05f };
            XMFLOAT3 length = {segmentLength, 0.04f, 0.04f};
            
            skeleton.AddBone(CreateBone(name, position, length, parentIdx), parentIdx);
            parentIdx = static_cast<int>(skeleton.GetBoneCount()) - 1;
        }
    }
}

} // namespace Animation
} // namespace Engine
