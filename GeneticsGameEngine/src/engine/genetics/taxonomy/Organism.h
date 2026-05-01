#pragma once

#include "../genome/Genome.h"
#include <string>
#include <vector>
#include <memory>

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Base abstract class for all organisms
class Organism {
protected:
    Genome m_genome;
    std::string m_speciesName;
    float m_scale;
    int m_colorIndex;
    std::string m_organismID;
    
public:
    Organism() : m_scale(1.0f), m_colorIndex(0), m_organismID("") {}
    virtual ~Organism() = default;
    
    // Pure virtual methods for genetic operations
    virtual void ApplyGeneticExpression(const Genome& genome) = 0;
    virtual std::vector<Genome> BreedWith(const Organism& other) const = 0;
    virtual void ApplyMutation(float mutationRate) = 0;
    
    // Getters
    const Genome& GetGenome() const { return m_genome; }
    const std::string& GetSpeciesName() const { return m_speciesName; }
    float GetScale() const { return m_scale; }
    int GetColorIndex() const { return m_colorIndex; }
    const std::string& GetID() const { return m_organismID; }
    
    // Setters
    void SetID(const std::string& id) { m_organismID = id; }
    void SetScale(float scale) { m_scale = scale; }
    void SetColorIndex(int colorIndex) { m_colorIndex = colorIndex; }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
