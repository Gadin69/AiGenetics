#pragma once

#include "Animal.h"

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Arthropoda - invertebrate animals with exoskeleton
class Arthropoda : public Animal {
private:
    float m_exoskeletonThickness;
    int m_segmentCount;
    float m_jointFlexibility;
    
public:
    Arthropoda() : m_exoskeletonThickness(1.0f), m_segmentCount(8), m_jointFlexibility(0.7f) {
        m_speciesName = "Arthropoda";
    }
    
    // Getters
    float GetExoskeletonThickness() const { return m_exoskeletonThickness; }
    int GetSegmentCount() const { return m_segmentCount; }
    float GetJointFlexibility() const { return m_jointFlexibility; }
    
    // Apply genetic expression specific to Arthropoda
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
        
        // Locus 0x5E6F: Limb count (2-12 for arthropods)
        const Gene* limbGene = genome.GetGene(0x5E6F);
        if (limbGene) {
            m_limbCount = 2 + static_cast<int>(limbGene->ExpressValue() * 10.99f); // Range: 2-12
            m_limbCount = std::clamp(m_limbCount, 2, 12);
        }
        
        // Arthropoda-specific genes
        const Gene* exoskeletonGene = genome.GetGene(0x3A4B);
        if (exoskeletonGene) {
            m_exoskeletonThickness = 0.3f + exoskeletonGene->ExpressValue() * 2.7f; // Range: 0.3-3.0
        }
        
        const Gene* segmentGene = genome.GetGene(0x5C6D);
        if (segmentGene) {
            m_segmentCount = 3 + static_cast<int>(segmentGene->ExpressValue() * 12.99f); // Range: 3-15
            m_segmentCount = std::clamp(m_segmentCount, 3, 15);
        }
        
        const Gene* jointGene = genome.GetGene(0x7E8F);
        if (jointGene) {
            m_jointFlexibility = jointGene->ExpressValue(); // Range: 0.0-1.0
        }
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
