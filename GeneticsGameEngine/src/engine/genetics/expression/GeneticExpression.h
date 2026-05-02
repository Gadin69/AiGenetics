#pragma once

#include "../genome/Genome.h"
#include "../taxonomy/Organism.h"
#include "ExpressionRules.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <typeinfo>

namespace Engine {
namespace Genetics {
namespace Expression {

// Genetic expression system that maps genetic data to visual parameters
// Implements the specification from PLAN.md sections 96-104
class GeneticExpression {
public:
    // Apply genetic expression to an organism based on its genome
    static void ApplyToOrganism(Engine::Genetics::Taxonomy::Organism& organism, const Engine::Genetics::Genome& genome) {
        // Get appropriate expression rules based on organism type
        const auto& rules = GetExpressionRulesForOrganism(organism);
        
        // Apply each rule
        for (const auto& rule : rules) {
            const Engine::Genetics::Gene* gene = genome.GetGene(rule.locusID);
            if (gene) {
                float parameterValue = rule.minValue + 
                                     gene->ExpressValue() * (rule.maxValue - rule.minValue);
                
                // Set the parameter based on its name
                SetOrganismParameter(organism, rule.parameterName, parameterValue);
            }
        }
    }
    
    // Get expression rules appropriate for the given organism
    static const std::vector<ExpressionRule>& GetExpressionRulesForOrganism(
        const Engine::Genetics::Taxonomy::Organism& organism) {
        
        // Determine organism type and return appropriate rules
        const std::string& species = organism.GetSpeciesName();
        
        if (species == "Chordata") {
            return ExpressionRules::GetChordataRules();
        } else if (species == "Arthropoda") {
            return ExpressionRules::GetArthropodaRules();
        } else if (species == "Mollusca") {
            return ExpressionRules::GetMolluscaRules();
        } else {
            // Default to generic rules
            return ExpressionRules::GetGenericRules();
        }
    }
    
private:
    // Helper method to set organism parameters by name
    static void SetOrganismParameter(Engine::Genetics::Taxonomy::Organism& organism, 
                                   const std::string& parameterName, 
                                   float value) {
        // Use base Organism interface - all subclasses inherit these methods
        if (parameterName == "scale") {
            organism.SetScale(value);
        } else if (parameterName == "colorIndex") {
            organism.SetColorIndex(static_cast<int>(value));
        }
        // Type-specific parameters (limbCount, tentacleCount, segmentCount, etc.) 
        // should be handled by the organism's ApplyGeneticExpression override
    }
};

} // namespace Expression
} // namespace Genetics
} // namespace Engine