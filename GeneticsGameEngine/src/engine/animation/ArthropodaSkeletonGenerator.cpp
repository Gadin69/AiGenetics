#include "ArthropodaSkeletonGenerator.h"
#include <cmath>

using namespace DirectX;

namespace Engine {
namespace Animation {

Skeleton ArthropodaSkeletonGenerator::GenerateSkeleton(
    const Engine::Genetics::Genome& genome,
    const Engine::Procedural::Generation::CreatureParams& params)
{
    Skeleton skeleton;
    
    // Genetic parameters for arthropod body plan
    float headSize = 0.4f * params.scaleFactor;
    int thoraxSegments = 3 + (static_cast<int>(genome.GetGeneValue(0x1100) * 100.0f) % 4); // 3-6 thorax segments
    int abdomenSegments = 4 + (static_cast<int>(genome.GetGeneValue(0x1101) * 100.0f) % 5); // 4-8 abdomen segments
    float segmentLength = 0.3f * params.scaleFactor;
    
    // STEP 1: Generate head
    GenerateHeadSegment(skeleton, headSize);
    
    // STEP 2: Generate thorax segments
    GenerateThoraxSegments(skeleton, thoraxSegments, segmentLength);
    
    // STEP 3: Generate abdomen segments
    GenerateAbdomenSegments(skeleton, abdomenSegments, segmentLength * 0.8f);
    
    // STEP 4: Generate legs for thorax segments (gene-driven!)
    // Arthropods typically have 3-8 leg pairs
    int legPairs = params.limbCount / 2;
    for (int i = 0; i < thoraxSegments && i < legPairs; ++i)
    {
        float legLength = 0.8f * params.scaleFactor;
        GenerateLegPair(skeleton, i + 1, legLength, i); // +1 because head is index 0
    }
    
    // STEP 5: Generate wings/appendages on specific thorax segments (gene-driven!)
    // Check genes for wing presence on each thorax segment
    for (int i = 0; i < thoraxSegments; ++i)
    {
        int wingGeneLocus = 0x1200 + i;
        int wingValue = static_cast<int>(genome.GetGeneValue(wingGeneLocus) * 100.0f);
        
        if (wingValue > 50 && i >= 1) // Wings on segments 2+
        {
            float wingLength = 1.2f * params.scaleFactor;
            GenerateWingPair(skeleton, i + 1, wingLength, i);
        }
    }
    
    // Compute final world transforms
    skeleton.ComputeWorldTransforms();
    
    return skeleton;
}

void ArthropodaSkeletonGenerator::GenerateHeadSegment(Skeleton& skeleton, float headSize)
{
    XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
    XMFLOAT3 length = {headSize * 1.2f, headSize * 0.9f, headSize * 1.0f}; // Thicker head
    
    skeleton.AddBone(CreateBone("Head", position, length, -1), -1);
}

void ArthropodaSkeletonGenerator::GenerateThoraxSegments(
    Skeleton& skeleton, int segmentCount, float segmentLength)
{
    for (int i = 0; i < segmentCount; ++i)
    {
        std::string name = "Thorax_" + std::to_string(i);
        XMFLOAT3 position = {0.0f, -segmentLength, 0.0f}; // Relative offset from parent (DOWN in Y)
        XMFLOAT3 length = {0.35f * segmentLength, segmentLength * 0.95f, 0.25f * segmentLength}; // Thicker segments
        
        int parentIndex = i; // Head is index 0, first thorax is index 1
        skeleton.AddBone(CreateBone(name, position, length, parentIndex), parentIndex);
    }
}

void ArthropodaSkeletonGenerator::GenerateAbdomenSegments(
    Skeleton& skeleton, int segmentCount, float segmentLength)
{
    // Abdomen attaches to last thorax segment
    int parentIndex = segmentCount; // Last thorax index
    
    for (int i = 0; i < segmentCount; ++i)
    {
        std::string name = "Abdomen_" + std::to_string(i);
        XMFLOAT3 position = {0.0f, -segmentLength, 0.0f}; // Down in Y
        XMFLOAT3 length = {0.18f * segmentLength, segmentLength * 0.9f, 0.12f * segmentLength}; // Y is length
        
        skeleton.AddBone(CreateBone(name, position, length, parentIndex), parentIndex);
        parentIndex = static_cast<int>(skeleton.GetBoneCount()) - 1;
    }
}

void ArthropodaSkeletonGenerator::GenerateLegPair(
    Skeleton& skeleton, int attachmentIndex, float legLength, int legIndex)
{
    std::string side;
    
    // Left leg
    {
        std::string name = "Leg_Left_" + std::to_string(legIndex);
        XMFLOAT3 position = {0.25f, -0.05f, 0.0f};
        XMFLOAT3 length = {0.05f, 0.05f, legLength};
        
        skeleton.AddBone(CreateBone(name, position, length, attachmentIndex), attachmentIndex);
    }
    
    // Right leg
    {
        std::string name = "Leg_Right_" + std::to_string(legIndex);
        XMFLOAT3 position = {-0.25f, -0.05f, 0.0f};
        XMFLOAT3 length = {0.05f, 0.05f, legLength};
        
        skeleton.AddBone(CreateBone(name, position, length, attachmentIndex), attachmentIndex);
    }
}

void ArthropodaSkeletonGenerator::GenerateWingPair(
    Skeleton& skeleton, int attachmentIndex, float wingLength, int wingIndex)
{
    // Left wing
    {
        std::string name = "Wing_Left_" + std::to_string(wingIndex);
        XMFLOAT3 position = {0.3f, 0.1f, 0.0f};
        XMFLOAT3 length = {0.03f, 0.03f, wingLength};
        
        skeleton.AddBone(CreateBone(name, position, length, attachmentIndex), attachmentIndex);
    }
    
    // Right wing
    {
        std::string name = "Wing_Right_" + std::to_string(wingIndex);
        XMFLOAT3 position = {-0.3f, 0.1f, 0.0f};
        XMFLOAT3 length = {0.03f, 0.03f, wingLength};
        
        skeleton.AddBone(CreateBone(name, position, length, attachmentIndex), attachmentIndex);
    }
}

} // namespace Animation
} // namespace Engine
