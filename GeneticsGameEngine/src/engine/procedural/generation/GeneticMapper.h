#pragma once

#include "CreatureParams.h"
#include "../../genetics/genome/Genome.h"

namespace Engine {
namespace Procedural {
namespace Generation {

// Maps genetic loci to creature visual parameters
// From tables.txt TABLE 12 (lines 633-644): Complete genetic loci mapping
class GeneticMapper {
public:
    GeneticMapper() = default;
    
    // Map genome to creature parameters
    // taxonomyType: 0=Chordata, 1=Arthropoda, 2=Mollusca
    CreatureParams MapGenomeToParams(const Genetics::Genome& genome, int taxonomyType = 0);
    
private:
    // Read gene value from genome (returns 0.0f if gene not found)
    float GetGeneValue(const Genetics::Genome& genome, uint16_t locusID) const;
    
    // Taxonomy-specific parameter adjustments
    void ApplyChordataParams(CreatureParams& params, const Genetics::Genome& genome) const;
    void ApplyArthropodaParams(CreatureParams& params, const Genetics::Genome& genome) const;
    void ApplyMolluscaParams(CreatureParams& params, const Genetics::Genome& genome) const;
};

} // namespace Generation
} // namespace Procedural
} // namespace Engine
