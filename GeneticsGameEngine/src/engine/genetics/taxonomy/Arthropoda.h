#pragma once

#include "./Animal.h"
#include "../expression/GeneticExpression.h"
#include "../../animation/Skeleton.h"
#include "../../animation/ArthropodaSkeletonGenerator.h"
#include <memory>

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Arthropoda - invertebrate animals with exoskeleton
class Arthropoda : public ::Engine::Genetics::Taxonomy::Animal {
private:
    float m_exoskeletonThickness;
    int m_segmentCount;
    float m_jointFlexibility;
    
public:
    Arthropoda() : m_exoskeletonThickness(1.0f), m_segmentCount(8), m_jointFlexibility(0.7f) {
        Organism::m_speciesName = "Arthropoda";
    }
    
    // Getters
    float GetExoskeletonThickness() const { return m_exoskeletonThickness; }
    int GetSegmentCount() const { return m_segmentCount; }
    float GetJointFlexibility() const { return m_jointFlexibility; }
    
    // Setters (inherited from Organism)
    void SetScale(float scale) { m_scale = scale; }
    void SetColorIndex(int colorIndex) { m_colorIndex = colorIndex; }
    void SetLimbCount(int limbs) { m_limbCount = limbs; }
    
    // Apply genetic expression specific to Arthropoda
    void ApplyGeneticExpression(const Genome& genome) override {
        // Use the centralized GeneticExpression system
        Engine::Genetics::Expression::GeneticExpression::ApplyToOrganism(*this, genome);
    }
    
    // NEW: Generate skeleton from genetics (polymorphic)
    std::unique_ptr<Engine::Animation::Skeleton> GenerateSkeleton() const {
        Engine::Animation::ArthropodaSkeletonGenerator generator;
        
        // Build creature params from genetic data
        Engine::Procedural::Generation::CreatureParams params;
        params.scaleFactor = m_scale;
        params.limbCount = m_limbCount;
        params.archetype = Engine::Procedural::Generation::ArchetypeType::Arthropoda;
        params.blendSmoothness = 0.05f; // Hard blending for exoskeleton
        
        return std::make_unique<Engine::Animation::Skeleton>(
            generator.GenerateSkeleton(m_genome, params)
        );
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
