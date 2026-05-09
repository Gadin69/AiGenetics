#pragma once

#include "SkeletonGenerator.h"

namespace Engine {
namespace Animation {

// Generates arthropod skeleton (insects, crustaceans)
// Structure: Head + Thorax segments + Abdomen + Leg pairs
class ArthropodaSkeletonGenerator : public SkeletonGenerator {
public:
    Skeleton GenerateSkeleton(const Engine::Genetics::Genome& genome, 
                              const Engine::Procedural::Generation::CreatureParams& params) override;
    
private:
    void GenerateHeadSegment(Skeleton& skeleton, float headSize);
    void GenerateThoraxSegments(Skeleton& skeleton, int segmentCount, float segmentLength);
    void GenerateAbdomenSegments(Skeleton& skeleton, int segmentCount, float segmentLength);
    void GenerateLegPair(Skeleton& skeleton, int attachmentIndex, float legLength, int legIndex);
    void GenerateWingPair(Skeleton& skeleton, int attachmentIndex, float wingLength, int wingIndex);
};

} // namespace Animation
} // namespace Engine
