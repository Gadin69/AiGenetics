#pragma once

#include "./Animal.h"
#include "../expression/GeneticExpression.h"

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Chordata - vertebrate animals with backbone
class Chordata : public ::Engine::Genetics::Taxonomy::Animal {
private:
    float m_skeletalDensity;
    float m_skinRoughness;
    float m_skinMetallic;
    
public:
    Chordata() : m_skeletalDensity(1.0f), m_skinRoughness(0.7f), m_skinMetallic(0.0f) {
        Organism::m_speciesName = "Chordata";
    }
    
    // Getters
    float GetSkeletalDensity() const { return m_skeletalDensity; }
    float GetSkinRoughness() const { return m_skinRoughness; }
    float GetSkinMetallic() const { return m_skinMetallic; }
    
    // Setters (inherited from Organism)
    void SetScale(float scale) { m_scale = scale; }
    void SetColorIndex(int colorIndex) { m_colorIndex = colorIndex; }
    void SetLimbCount(int limbs) { m_limbCount = limbs; }
    
    // Apply genetic expression specific to Chordata
    void ApplyGeneticExpression(const Genome& genome) override {
        // Use the centralized GeneticExpression system
        Engine::Genetics::Expression::GeneticExpression::ApplyToOrganism(*this, genome);
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
