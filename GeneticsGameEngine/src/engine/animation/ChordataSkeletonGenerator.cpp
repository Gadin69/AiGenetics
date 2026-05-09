#include "ChordataSkeletonGenerator.h"
#include <cmath>

using namespace DirectX;

namespace Engine {
namespace Animation {

Skeleton ChordataSkeletonGenerator::GenerateSkeleton(
    const Engine::Genetics::Genome& genome,
    const Engine::Procedural::Generation::CreatureParams& params)
{
    Skeleton skeleton;
    
    // Extract genetic parameters
    float bodyLength = 2.0f * params.scaleFactor;
    int vertebraCount = 8 + (static_cast<int>(genome.GetGeneValue(0x1001) * 100.0f) % 8); // 8-15 vertebrae (gene-driven!)
    float headSize = 0.4f * params.scaleFactor;
    
    // STEP 1: Generate spine (central axis)
    GenerateSpine(skeleton, bodyLength, vertebraCount);
    
    // STEP 2: Generate head at top of spine
    GenerateHead(skeleton, vertebraCount - 1, headSize);
    
    // STEP 3: Generate attachments for EACH vertebra (gene-driven!)
    // This is where genetic diversity explodes - each vertebra independently decides
    // what to grow based on genes, not hardcoded templates
    for (int i = 0; i < vertebraCount; ++i)
    {
        GenerateAttachmentsForVertebra(skeleton, i, genome, i, vertebraCount);
    }
    
    // Compute final world transforms
    skeleton.ComputeWorldTransforms();
    
    return skeleton;
}

void ChordataSkeletonGenerator::GenerateSpine(Skeleton& skeleton, float bodyLength, int vertebraCount)
{
    float vertebraSpacing = bodyLength / vertebraCount;
    
    // Create vertebrae from bottom to top (along Y axis)
    for (int i = 0; i < vertebraCount; ++i)
    {
        std::string name = "Vertebra_" + std::to_string(i);
        
        // First vertebra is root - use absolute position
        // Subsequent vertebrae use RELATIVE offset from parent
        XMFLOAT3 position;
        if (i == 0)
        {
            position = {0.0f, -bodyLength / 2.0f, 0.0f}; // Start at bottom (negative Y)
        }
        else
        {
            // Relative offset from parent (move UP in Y)
            position = {0.0f, vertebraSpacing, 0.0f};
        }
        
        XMFLOAT3 length = {0.15f, vertebraSpacing * 0.8f, 0.15f}; // Y is the length axis
        
        int parentIndex = (i > 0) ? (i - 1) : -1; // First vertebra is root
        skeleton.AddBone(CreateBone(name, position, length, parentIndex), parentIndex);
    }
}

void ChordataSkeletonGenerator::GenerateHead(Skeleton& skeleton, int spineTopIndex, float headSize)
{
    XMFLOAT3 position = {0.0f, headSize * 0.5f, 0.0f}; // Above spine top (Y axis)
    XMFLOAT3 length = {headSize, headSize * 0.8f, headSize};
    
    skeleton.AddBone(CreateBone("Head", position, length, spineTopIndex), spineTopIndex);
}

// NEW: Flexible attachment system - each vertebra can grow up to 5 appendages
void ChordataSkeletonGenerator::GenerateAttachmentsForVertebra(
    Skeleton& skeleton, int vertebraIndex,
    const Engine::Genetics::Genome& genome,
    int vertebraPosition, int totalVertebrae)
{
    // Find the vertebra bone index (head is at spineTopIndex+1, so vertebrae are 0..totalVertebrae-1)
    int vertebraBoneIndex = vertebraIndex;
    
    // Get vertebra's world position for length calculations
    float normalizedPos = static_cast<float>(vertebraPosition) / totalVertebrae;
    float baseLength = 0.5f + normalizedPos * 1.0f; // Varies along spine
    
    // Check each of the 5 attachment slots
    for (int slotInt = 0; slotInt < static_cast<int>(AttachmentSlot::COUNT); ++slotInt)
    {
        AttachmentSlot slot = static_cast<AttachmentSlot>(slotInt);
        
        // Determine what grows here based on genes
        AppendageType appendageType = DetermineAppendageType(genome, vertebraIndex, slot);
        
        if (appendageType != AppendageType::NONE)
        {
            GenerateAppendage(skeleton, vertebraBoneIndex, appendageType, slot, 
                            baseLength, vertebraIndex);
        }
    }
}

// Gene-driven appendage determination - THIS IS WHERE EVOLUTION HAPPENS
AppendageType ChordataSkeletonGenerator::DetermineAppendageType(
    const Engine::Genetics::Genome& genome,
    int vertebraIndex, AttachmentSlot slot) const
{
    // Each slot uses a different gene locus to determine appendage type
    // This allows independent evolution of each attachment point
    int geneLocus = 0x2000 + (vertebraIndex * 10) + static_cast<int>(slot);
    
    // Gene value 0-9 determines appendage type
    int geneValue = static_cast<int>(genome.GetGeneValue(geneLocus) * 100.0f); // 0-99
    
    // Probability distribution (can be modified by taxonomy):
    // 0-29: NONE (30% chance - most slots empty)
    // 30-49: LEG (20%)
    // 50-59: ARM (10%)
    // 60-64: WING (5%)
    // 65-69: EYE (5%)
    // 70-74: ANTENNA (5%)
    // 75-79: FIN (5%)
    // 80-84: CLAW (5%)
    // 85-89: TAIL_EXT (5%)
    // 90-94: ORGAN (5%)
    
    if (geneValue < 30) return AppendageType::NONE;
    if (geneValue < 50) return AppendageType::LEG;
    if (geneValue < 60) return AppendageType::ARM;
    if (geneValue < 65) return AppendageType::WING;
    if (geneValue < 70) return AppendageType::EYE;
    if (geneValue < 75) return AppendageType::ANTENNA;
    if (geneValue < 80) return AppendageType::FIN;
    if (geneValue < 85) return AppendageType::CLAW;
    if (geneValue < 90) return AppendageType::TAIL_EXT;
    return AppendageType::ORGAN;
}

void ChordataSkeletonGenerator::GenerateAppendage(
    Skeleton& skeleton, int parentBoneIndex,
    AppendageType type, AttachmentSlot slot,
    float length, int vertebraIndex)
{
    std::string typeName;
    XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
    XMFLOAT3 boneLength = {0.1f, 0.1f, length};
    float rotation = 0.0f;
    
    // Position based on attachment slot (pentagonal arrangement)
    switch (slot)
    {
    case AttachmentSlot::DORSAL:
        position = {0.0f, 0.2f, 0.0f}; // Top
        rotation = 0.0f;
        break;
    case AttachmentSlot::LEFT_LATERAL:
        position = {0.25f, 0.0f, 0.0f}; // Left side
        rotation = XM_PIDIV2; // 90 degrees
        break;
    case AttachmentSlot::RIGHT_LATERAL:
        position = {-0.25f, 0.0f, 0.0f}; // Right side
        rotation = -XM_PIDIV2;
        break;
    case AttachmentSlot::VENTRAL_LEFT:
        position = {0.15f, -0.15f, 0.0f}; // Bottom-left
        rotation = XM_PIDIV4 * 3.0f;
        break;
    case AttachmentSlot::VENTRAL_RIGHT:
        position = {-0.15f, -0.15f, 0.0f}; // Bottom-right
        rotation = -XM_PIDIV4 * 3.0f;
        break;
    }
    
    // Generate based on type
    switch (type)
    {
    case AppendageType::LEG:
    case AppendageType::ARM:
        typeName = (type == AppendageType::LEG) ? "Leg" : "Arm";
        {
            std::string name = typeName + "_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::WING:
        {
            std::string name = "Wing_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            // Wings are longer and thinner
            boneLength = {0.05f, 0.05f, length * 2.0f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::EYE:
        {
            std::string name = "Eye_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            XMFLOAT3 size = {0.1f, 0.1f, 0.1f};
            GenerateSensoryOrgan(skeleton, name, parentBoneIndex, position, size);
        }
        break;
        
    case AppendageType::ANTENNA:
        {
            std::string name = "Antenna_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            // Antennae are thin and long
            boneLength = {0.02f, 0.02f, length * 1.5f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::FIN:
        {
            std::string name = "Fin_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            // Fins are flat and wide
            boneLength = {0.3f, 0.05f, length * 0.8f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::CLAW:
        {
            std::string name = "Claw_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            boneLength = {0.08f, 0.08f, length * 0.6f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::TAIL_EXT:
        {
            std::string name = "TailExt_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            boneLength = {0.08f, 0.08f, length * 1.2f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::ORGAN:
        {
            std::string name = "Organ_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            XMFLOAT3 size = {0.2f, 0.2f, 0.2f};
            GenerateSensoryOrgan(skeleton, name, parentBoneIndex, position, size);
        }
        break;
        
    default:
        break;
    }
}

void ChordataSkeletonGenerator::GenerateLimbBone(
    Skeleton& skeleton, const std::string& name,
    int parentIndex, DirectX::XMFLOAT3 position,
    DirectX::XMFLOAT3 length, float rotationOffset)
{
    Bone bone;
    bone.name = name;
    bone.localPosition = position;
    bone.boneLength = length;
    bone.localRotation = {rotationOffset, 0.0f, 0.0f};
    bone.parentIndex = parentIndex;
    bone.mass = length.x * length.y * length.z;
    
    skeleton.AddBone(bone, parentIndex);
}

void ChordataSkeletonGenerator::GenerateSensoryOrgan(
    Skeleton& skeleton, const std::string& name,
    int parentIndex, DirectX::XMFLOAT3 position,
    DirectX::XMFLOAT3 size)
{
    Bone bone;
    bone.name = name;
    bone.localPosition = position;
    bone.boneLength = size;
    bone.localRotation = {0.0f, 0.0f, 0.0f};
    bone.parentIndex = parentIndex;
    bone.mass = size.x * size.y * size.z * 0.5f; // Organs lighter
    
    skeleton.AddBone(bone, parentIndex);
}

} // namespace Animation
} // namespace Engine
