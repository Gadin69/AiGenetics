#pragma once

#include "../genome/Gene.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace Engine {
namespace Genetics {
namespace Expression {

// Structure to define how a genetic locus maps to visual parameters
struct ExpressionRule {
    uint16_t locusID;                    // Genetic locus identifier
    std::string parameterName;           // Name of the visual parameter
    float minValue;                      // Minimum value for this parameter
    float maxValue;                      // Maximum value for this parameter
    std::string description;             // Description of what this locus controls
    
    ExpressionRule(uint16_t id, const std::string& param, float minVal, float maxVal, 
                   const std::string& desc) 
        : locusID(id), parameterName(param), minValue(minVal), maxValue(maxVal), description(desc) {}
};

// Global expression rules registry
// These match the specifications in PLAN.md sections 96-104
class ExpressionRules {
public:
    // Chordata-specific expression rules
    static const std::vector<ExpressionRule>& GetChordataRules() {
        static std::vector<ExpressionRule> rules = {
            {0x1A2B, "scale", 0.5f, 3.0f, "Controls scale factor"},
            {0x3C4D, "colorIndex", 0.0f, 7.0f, "Controls color palette index"},
            {0x5E6F, "limbCount", 1.0f, 8.0f, "Controls limb count"},
            {0x7A8B, "skeletalDensity", 0.5f, 2.0f, "Controls skeletal density"},
            {0x9C0D, "skinRoughness", 0.0f, 1.0f, "Controls skin roughness"},
            {0x1E2F, "skinMetallic", 0.0f, 0.3f, "Controls skin metallic property"}
        };
        return rules;
    }
    
    // Arthropoda-specific expression rules
    static const std::vector<ExpressionRule>& GetArthropodaRules() {
        static std::vector<ExpressionRule> rules = {
            {0x1A2B, "scale", 0.5f, 3.0f, "Controls scale factor"},
            {0x3C4D, "colorIndex", 0.0f, 7.0f, "Controls color palette index"},
            {0x5E6F, "limbCount", 2.0f, 12.0f, "Controls limb count"},
            {0x3A4B, "exoskeletonThickness", 0.3f, 3.0f, "Controls exoskeleton thickness"},
            {0x5C6D, "segmentCount", 3.0f, 15.0f, "Controls segment count"},
            {0x7E8F, "jointFlexibility", 0.0f, 1.0f, "Controls joint flexibility"}
        };
        return rules;
    }
    
    // Mollusca-specific expression rules
    static const std::vector<ExpressionRule>& GetMolluscaRules() {
        static std::vector<ExpressionRule> rules = {
            {0x1A2B, "scale", 0.5f, 3.0f, "Controls scale factor"},
            {0x3C4D, "colorIndex", 0.0f, 7.0f, "Controls color palette index"},
            {0x5E6F, "tentacleCount", 0.0f, 8.0f, "Controls tentacle count"},
            {0x9A0B, "shellSpiralAngle", 10.0f, 90.0f, "Controls shell spiral angle"},
            {0x1C2D, "shellThickness", 0.0f, 1.0f, "Controls shell thickness"},
            {0x3E4F, "mantleTexture", 0.0f, 1.0f, "Controls mantle texture"}
        };
        return rules;
    }
    
    // Generic expression rules that apply to all taxa
    static const std::vector<ExpressionRule>& GetGenericRules() {
        static std::vector<ExpressionRule> rules = {
            {0x1A2B, "scale", 0.5f, 3.0f, "Controls scale factor"},
            {0x3C4D, "colorIndex", 0.0f, 7.0f, "Controls color palette index"},
            {0x5E6F, "limbCount", 0.0f, 8.0f, "Controls limb count"}
        };
        return rules;
    }
};

} // namespace Expression
} // namespace Genetics
} // namespace Engine