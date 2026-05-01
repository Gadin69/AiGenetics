#pragma once

#include "../genome/Genome.h"
#include "../taxonomy/Organism.h"
#include "ExpressionRules.h"
#include <DirectXMath.h>

namespace Engine {
namespace Genetics {
namespace Expression {

// Visual parameters ready for rendering
struct VisualParameters {
    float scale;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    int limbCount;
    int segmentCount;
    float shellSpiralAngle;
    float exoskeletonThickness;
    
    VisualParameters() 
        : scale(1.0f), 
          position(0.0f, 0.0f, 0.0f), 
          color(1.0f, 1.0f, 1.0f, 1.0f),
          limbCount(4), 
          segmentCount(8),
          shellSpiralAngle(45.0f),
          exoskeletonThickness(1.0f) {}
};

// Genetic expression engine
class GeneticExpressionEngine {
public:
    // Apply genetic expression to an organism
    static void ApplyExpression(Taxonomy::Organism& organism, const Genome& genome) {
        organism.ApplyGeneticExpression(genome);
    }
    
    // Extract visual parameters from a genome for rendering
    static VisualParameters ExtractVisualParameters(const Genome& genome) {
        VisualParameters params;
        
        // Extract scale (Locus 0x1A2B: 0.5-3.0)
        const Gene* scaleGene = genome.GetGene(0x1A2B);
        if (scaleGene) {
            ExpressionRule scaleRule(0x1A2B, 0.5f, 3.0f, ParameterType::Scale);
            params.scale = scaleRule.ConvertGeneValue(scaleGene->ExpressValue());
        }
        
        // Extract color (Locus 0x3C4D: color palette index 0-7)
        const Gene* colorGene = genome.GetGene(0x3C4D);
        if (colorGene) {
            int colorIndex = static_cast<int>(colorGene->ExpressValue() * 7.99f);
            colorIndex = std::clamp(colorIndex, 0, 7);
            params.color = GetColorFromIndex(colorIndex);
        }
        
        // Extract limb count (Locus 0x5E6F: 1-8)
        const Gene* limbGene = genome.GetGene(0x5E6F);
        if (limbGene) {
            ExpressionRule limbRule(0x5E6F, 1.0f, 8.0f, ParameterType::LimbCount);
            float limbValue = limbRule.ConvertGeneValue(limbGene->ExpressValue());
            params.limbCount = static_cast<int>(limbValue);
        }
        
        // Extract segment count for Arthropoda (Locus 0x5C6D: 3-15)
        const Gene* segmentGene = genome.GetGene(0x5C6D);
        if (segmentGene) {
            ExpressionRule segmentRule(0x5C6D, 3.0f, 15.0f, ParameterType::SegmentCount);
            float segmentValue = segmentRule.ConvertGeneValue(segmentGene->ExpressValue());
            params.segmentCount = static_cast<int>(segmentValue);
        }
        
        // Extract shell spiral for Mollusca (Locus 0x9A0B: 10-90 degrees)
        const Gene* spiralGene = genome.GetGene(0x9A0B);
        if (spiralGene) {
            ExpressionRule spiralRule(0x9A0B, 10.0f, 90.0f, ParameterType::ShellSpiral);
            params.shellSpiralAngle = spiralRule.ConvertGeneValue(spiralGene->ExpressValue());
        }
        
        // Extract exoskeleton thickness (Locus 0x3A4B: 0.3-3.0)
        const Gene* exoskeletonGene = genome.GetGene(0x3A4B);
        if (exoskeletonGene) {
            ExpressionRule exoskeletonRule(0x3A4B, 0.3f, 3.0f, ParameterType::ExoskeletonThickness);
            params.exoskeletonThickness = exoskeletonRule.ConvertGeneValue(exoskeletonGene->ExpressValue());
        }
        
        return params;
    }
    
private:
    // Color palette for genetic color expression
    static DirectX::XMFLOAT4 GetColorFromIndex(int index) {
        static const DirectX::XMFLOAT4 colorPalette[] = {
            DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    // Red
            DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),    // Green
            DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),    // Blue
            DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),    // Yellow
            DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),    // Magenta
            DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f),    // Cyan
            DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),    // Orange
            DirectX::XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f)     // Purple
        };
        
        if (index >= 0 && index < 8) {
            return colorPalette[index];
        }
        return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
    }
};

} // namespace Expression
} // namespace Genetics
} // namespace Engine
