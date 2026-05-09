#pragma once

#include "../animation/Skeleton.h"
#include "../genetics/genome/Genome.h"
#include "../procedural/generation/CreatureParams.h"
#include <memory>

namespace Engine {
namespace Animation {

// Abstract base class for skeleton generation
class SkeletonGenerator {
public:
    virtual ~SkeletonGenerator() = default;
    virtual Skeleton GenerateSkeleton(const Engine::Genetics::Genome& genome, 
                                      const Engine::Procedural::Generation::CreatureParams& params) = 0;
    
protected:
    // Helper to create a bone with common defaults
    Bone CreateBone(const std::string& name, 
                    DirectX::XMFLOAT3 position, 
                    DirectX::XMFLOAT3 length,
                    int32_t parentIndex)
    {
        Bone bone;
        bone.name = name;
        bone.localPosition = position;
        bone.boneLength = length;
        bone.parentIndex = parentIndex;
        bone.mass = length.x * length.y * length.z; // Mass proportional to volume
        return bone;
    }
};

} // namespace Animation
} // namespace Engine
