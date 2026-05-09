#pragma once

#include "SkeletonGenerator.h"

namespace Engine {
namespace Animation {

// Generates mollusk skeleton (snails, octopuses, squids)
// Structure: Foot + Visceral mass + Shell (optional) + Tentacles
class MolluscaSkeletonGenerator : public SkeletonGenerator {
public:
    Skeleton GenerateSkeleton(const Engine::Genetics::Genome& genome, 
                              const Engine::Procedural::Generation::CreatureParams& params) override;
    
private:
    void GenerateFoot(Skeleton& skeleton, float footLength);
    void GenerateVisceralMass(Skeleton& skeleton, float massSize);
    void GenerateShell(Skeleton& skeleton, int spiralTurns, float shellSize);
    void GenerateTentaclePair(Skeleton& skeleton, int attachmentIndex, float tentacleLength, int tentacleIndex);
};

} // namespace Animation
} // namespace Engine
