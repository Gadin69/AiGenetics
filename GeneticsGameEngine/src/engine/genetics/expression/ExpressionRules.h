#pragma once

#include <cstdint>

namespace Engine {
namespace Genetics {
namespace Expression {

// Parameter types that genes can control
enum class ParameterType {
    Scale,
    Color,
    LimbCount,
    ExoskeletonThickness,
    SegmentCount,
    ShellSpiral,
    ShellThickness,
    SkeletalDensity,
    SkinRoughness,
    SkinMetallic
};

// Rule for mapping a genetic locus to a visual parameter
struct ExpressionRule {
    uint16_t locusID;
    float minValue;
    float maxValue;
    ParameterType type;
    
    ExpressionRule() : locusID(0), minValue(0.0f), maxValue(1.0f), type(ParameterType::Scale) {}
    
    ExpressionRule(uint16_t id, float min, float max, ParameterType t)
        : locusID(id), minValue(min), maxValue(max), type(t) {}
    
    // Convert gene value (0-1) to parameter range
    float ConvertGeneValue(float geneValue) const {
        return minValue + geneValue * (maxValue - minValue);
    }
};

} // namespace Expression
} // namespace Genetics
} // namespace Engine
