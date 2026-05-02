#pragma once

#include "./Animal.h"
#include "../expression/GeneticExpression.h"

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Mollusca - soft-bodied animals, often with shells
class Mollusca : public ::Engine::Genetics::Taxonomy::Animal {
private:
    float m_shellSpiralAngle;
    float m_shellThickness;
    float m_mantleTexture;
    
public:
    Mollusca() : m_shellSpiralAngle(45.0f), m_shellThickness(0.5f), m_mantleTexture(0.5f) {
        Organism::m_speciesName = "Mollusca";
    }
    
    // Getters
    float GetShellSpiralAngle() const { return m_shellSpiralAngle; }
    float GetShellThickness() const { return m_shellThickness; }
    float GetMantleTexture() const { return m_mantleTexture; }
    
    // Setters (inherited from Organism)
    void SetScale(float scale) { m_scale = scale; }
    void SetColorIndex(int colorIndex) { m_colorIndex = colorIndex; }
    void SetLimbCount(int limbs) { m_limbCount = limbs; }
    
    // Apply genetic expression specific to Mollusca
    void ApplyGeneticExpression(const Genome& genome) override {
        // Use the centralized GeneticExpression system
        Engine::Genetics::Expression::GeneticExpression::ApplyToOrganism(*this, genome);
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
