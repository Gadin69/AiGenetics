#pragma once

#include "Animal.h"

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Mollusca - soft-bodied animals, often with shells
class Mollusca : public Animal {
private:
    float m_shellSpiralAngle;
    float m_shellThickness;
    float m_mantleTexture;
    
public:
    Mollusca() : m_shellSpiralAngle(45.0f), m_shellThickness(0.5f), m_mantleTexture(0.5f) {
        m_speciesName = "Mollusca";
    }
    
    // Getters
    float GetShellSpiralAngle() const { return m_shellSpiralAngle; }
    float GetShellThickness() const { return m_shellThickness; }
    float GetMantleTexture() const { return m_mantleTexture; }
    
    // Apply genetic expression specific to Mollusca
    void ApplyGeneticExpression(const Genome& genome) override {
        m_genome = genome;
        
        // Map genes to visual parameters
        // Locus 0x1A2B: Scale factor (0.5-3.0)
        const Gene* scaleGene = genome.GetGene(0x1A2B);
        if (scaleGene) {
            m_scale = 0.5f + scaleGene->ExpressValue() * 2.5f; // Range: 0.5-3.0
        }
        
        // Locus 0x3C4D: Color palette index (0-7)
        const Gene* colorGene = genome.GetGene(0x3C4D);
        if (colorGene) {
            m_colorIndex = static_cast<int>(colorGene->ExpressValue() * 7.99f); // Range: 0-7
            m_colorIndex = std::clamp(m_colorIndex, 0, 7);
        }
        
        // Locus 0x5E6F: Tentacle count (0-8 for mollusks)
        const Gene* limbGene = genome.GetGene(0x5E6F);
        if (limbGene) {
            m_limbCount = static_cast<int>(limbGene->ExpressValue() * 8.99f); // Range: 0-8
            m_limbCount = std::clamp(m_limbCount, 0, 8);
        }
        
        // Mollusca-specific genes
        const Gene* spiralGene = genome.GetGene(0x9A0B);
        if (spiralGene) {
            m_shellSpiralAngle = 10.0f + spiralGene->ExpressValue() * 80.0f; // Range: 10-90 degrees
        }
        
        const Gene* shellGene = genome.GetGene(0x1C2D);
        if (shellGene) {
            m_shellThickness = shellGene->ExpressValue(); // Range: 0.0-1.0
        }
        
        const Gene* mantleGene = genome.GetGene(0x3E4F);
        if (mantleGene) {
            m_mantleTexture = mantleGene->ExpressValue(); // Range: 0.0-1.0
        }
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
