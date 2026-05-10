#pragma once

#include "SkeletonGenerator.h"

namespace Engine {
namespace Animation {

// Appendage types that can grow from bone attachment points
enum class AppendageType {
    NONE = 0,
    LEG,           // Locomotion limb
    ARM,           // Manipulation limb
    WING,          // Flight appendage
    EYE,           // Sensory organ
    ANTENNA,       // Chemical sensor
    FIN,           // Aquatic appendage
    CLAW,          // Weapon/Tool
    TAIL_EXT,      // Tail extension
    ORGAN,         // Generic organ (hump, sac, etc.)
    COUNT          // Must be last
};

// Attachment point on a bone (pentagonal arrangement)
enum class AttachmentSlot {
    DORSAL = 0,    // Top (up)
    LEFT_LATERAL,  // Left side
    RIGHT_LATERAL, // Right side
    VENTRAL_LEFT,  // Bottom-left
    VENTRAL_RIGHT, // Bottom-right
    COUNT          // Must be last
};

// Generates vertebrate skeleton with flexible appendage system
// Each vertebra can have up to 5 attachment points (pentagonal formation)
// Genes determine what grows at each point - NO hardcoded body plans
class ChordataSkeletonGenerator : public SkeletonGenerator {
public:
    Skeleton GenerateSkeleton(const Engine::Genetics::Genome& genome, 
                              const Engine::Procedural::Generation::CreatureParams& params) override;
    
private:
    // Core body generation
    void GenerateSpine(Skeleton& skeleton, float bodyLength, int vertebraCount);
    void GenerateHead(Skeleton& skeleton, int spineTopIndex, float headSize);
    
    // NEW: Flexible attachment system
    void GenerateAttachmentsForVertebra(Skeleton& skeleton, int vertebraIndex,
                                        const Engine::Genetics::Genome& genome,
                                        int vertebraPosition, int totalVertebrae);
    
    AppendageType DetermineAppendageType(const Engine::Genetics::Genome& genome,
                                         int vertebraIndex, AttachmentSlot slot) const;
    
    void GenerateAppendage(Skeleton& skeleton, int parentBoneIndex,
                          AppendageType type, AttachmentSlot slot,
                          float length, int vertebraIndex,
                          const Engine::Genetics::Genome& genome);
    
    void GenerateLimbBone(Skeleton& skeleton, const std::string& name,
                         int parentIndex, DirectX::XMFLOAT3 position,
                         DirectX::XMFLOAT3 length, float rotationOffset = 0.0f);
    
    // NEW: Multi-segment limb generation
    int GetLimbSegmentCount(const Engine::Genetics::Genome& genome, AppendageType type) const;
    void GenerateMultiSegmentLimb(Skeleton& skeleton, const std::string& baseName,
                                  int parentIndex, DirectX::XMFLOAT3 startPosition,
                                  DirectX::XMFLOAT3 boneLength, float rotation,
                                  AppendageType type, int segmentCount, int vertebraIndex, int slotIndex,
                                  DirectX::XMFLOAT3 growthDirection);
    
    void GenerateSensoryOrgan(Skeleton& skeleton, const std::string& name,
                             int parentIndex, DirectX::XMFLOAT3 position,
                             DirectX::XMFLOAT3 size);
};

} // namespace Animation
} // namespace Engine
