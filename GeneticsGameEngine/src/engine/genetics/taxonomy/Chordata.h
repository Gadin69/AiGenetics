#pragma once

#include "Animal.h"

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Chordata - vertebrate animals with backbone
class Chordata : public Animal {
private:
    float m_skeletalDensity;
    float m_skinRoughness;
    float m_skinMetallic;
    
public:
    Chordata() : m_skeletalDensity(1.0f), m_skinRoughness(0.7f), m_skinMetallic(0.0f) {
        m_speciesName = "Chordata";
    }
    
    // Getters
    float GetSkeletalDensity() const { return m_skeletalDensity; }
    float GetSkinRoughness() const { return m_skinRoughness; }
    float GetSkinMetallic() const { return m_skinMetallic; }
    
    // Apply genetic expression specific to Chordata
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
        
        // Locus 0x5E6F: Limb count (1-8)
        const Gene* limbGene = genome.GetGene(0x5E6F);
        if (limbGene) {
            m_limbCount = 1 + static_cast<int>(limbGene->ExpressValue() * 7.99f); // Range: 1-8
            m_limbCount = std::clamp(m_limbCount, 1, 8);
        }
        
        // Additional Chordata-specific genes
        const Gene* skeletalGene = genome.GetGene(0x7A8B);
        if (skeletalGene) {
            m_skeletalDensity = 0.5f + skeletalGene->ExpressValue() * 1.5f;
        }
        
        const Gene* roughnessGene = genome.GetGene(0x9C0D);
        if (roughnessGene) {
            m_skinRoughness = roughnessGene->ExpressValue();
        }
        
        const Gene* metallicGene = genome.GetGene(0x1E2F);
        if (metallicGene) {
            m_skinMetallic = metallicGene->ExpressValue() * 0.3f; // Biological tissues rarely highly metallic
        }
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
