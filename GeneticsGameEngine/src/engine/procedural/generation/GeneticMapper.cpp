#include "GeneticMapper.h"
#include <algorithm>

namespace Engine {
namespace Procedural {
namespace Generation {

// Genetic locus IDs from tables.txt TABLE 12 (lines 633-644)
static const uint16_t LOCUS_SCALE = 0x1A2B;           // Scale factor 0.5-3.0
static const uint16_t LOCUS_COLOR = 0x3C4D;            // Color palette index 0-7
static const uint16_t LOCUS_LIMB_COUNT = 0x5E6F;       // Limb count 1-8
static const uint16_t LOCUS_VOXEL_RES = 0x7A8B;        // Voxel resolution 32/64/128
static const uint16_t LOCUS_SMOOTHNESS = 0x9C0D;       // Surface smoothness 0-100
static const uint16_t LOCUS_CAVITY = 0x1E2F;           // Internal cavity 0.0-1.0
static const uint16_t LOCUS_ROUGHNESS = 0x3A4B;        // PBR roughness 0.0-1.0
static const uint16_t LOCUS_METALLIC = 0x5C6D;         // PBR metallic 0.0-1.0
static const uint16_t LOCUS_SSS = 0x7E8F;              // Subsurface scattering 0.0-1.0

CreatureParams GeneticMapper::MapGenomeToParams(const Genetics::Genome& genome, int taxonomyType) {
    CreatureParams params;
    params.taxonomyType = taxonomyType;
    
    // Read gene values (normalized 0.0-1.0)
    float scaleGene = GetGeneValue(genome, LOCUS_SCALE);
    float colorGene = GetGeneValue(genome, LOCUS_COLOR);
    float limbGene = GetGeneValue(genome, LOCUS_LIMB_COUNT);
    float voxelResGene = GetGeneValue(genome, LOCUS_VOXEL_RES);
    float smoothGene = GetGeneValue(genome, LOCUS_SMOOTHNESS);
    float cavityGene = GetGeneValue(genome, LOCUS_CAVITY);
    float roughGene = GetGeneValue(genome, LOCUS_ROUGHNESS);
    float metalGene = GetGeneValue(genome, LOCUS_METALLIC);
    float sssGene = GetGeneValue(genome, LOCUS_SSS);
    
    // Map normalized gene values to parameter ranges
    // From tables.txt TABLE 12 (lines 633-644)
    
    // Scale: 0.5f - 3.0f
    params.scaleFactor = 0.5f + scaleGene * 2.5f;
    
    // Color palette index: 0-7
    params.colorPaletteIndex = static_cast<int>(colorGene * 7.99f); // 7.99 to get 0-7 range
    
    // Limb count: 1-8
    params.limbCount = 1 + static_cast<int>(limbGene * 7.99f);
    
    // Voxel resolution: 32/64/128
    if (voxelResGene < 0.33f) {
        params.voxelResolution = 32;
    } else if (voxelResGene < 0.66f) {
        params.voxelResolution = 64;
    } else {
        params.voxelResolution = 128;
    }
    
    // Surface smoothness: 0-100
    params.surfaceSmoothness = smoothGene * 100.0f;
    
    // Internal cavity: 0.0-1.0
    params.internalCavity = cavityGene;
    
    // PBR parameters: 0.0-1.0
    params.roughness = roughGene;
    params.metallic = metalGene;
    params.subsurfaceScattering = sssGene;
    
    // Apply taxonomy-specific parameters
    // From tables.txt TABLE 12 (lines 646-660) and TABLE 7
    switch (taxonomyType) {
        case 0: // Chordata
            ApplyChordataParams(params, genome);
            break;
        case 1: // Arthropoda
            ApplyArthropodaParams(params, genome);
            break;
        case 2: // Mollusca
            ApplyMolluscaParams(params, genome);
            break;
        default:
            ApplyChordataParams(params, genome); // Default to Chordata
            break;
    }
    
    return params;
}

float GeneticMapper::GetGeneValue(const Genetics::Genome& genome, uint16_t locusID) const {
    const Genetics::Gene* gene = genome.GetGene(locusID);
    if (gene) {
        return gene->ExpressValue();
    }
    return 0.5f; // Default value if gene not found
}

// Chordata: Scale, limb count, skin texture, skeletal structure
// From tables.txt TABLE 7 (lines 433-446) and TABLE 12 (lines 647-650)
void GeneticMapper::ApplyChordataParams(CreatureParams& params, const Genetics::Genome& genome) const {
    // Body proportions for vertebrates
    params.bodyCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    params.bodyRadii = DirectX::XMFLOAT3(
        params.scaleFactor * 1.0f,  // length
        params.scaleFactor * 0.5f,  // width
        params.scaleFactor * 0.5f   // height
    );
    
    // Head position (on top of body)
    params.headCenter = DirectX::XMFLOAT3(
        0.0f, 
        params.bodyRadii.y + 0.3f * params.scaleFactor, 
        0.0f
    );
    params.headRadius = 0.3f * params.scaleFactor;
    
    // Limb count mapping from TABLE 12 (line 649-650)
    // Gene value 0-63: 2 limbs, 64-127: 4 limbs, 128-191: 6 limbs, 192-255: 8 limbs
    // Already handled in main mapping (1-8 range)
}

// Arthropoda: Exoskeleton thickness, segment count, joint flexibility
// From tables.txt TABLE 7 (lines 448-460) and TABLE 12 (lines 652-655)
void GeneticMapper::ApplyArthropodaParams(CreatureParams& params, const Genetics::Genome& genome) const {
    // Body proportions for arthropods (segmented body)
    params.bodyCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    params.bodyRadii = DirectX::XMFLOAT3(
        params.scaleFactor * 1.5f,  // length (longer body)
        params.scaleFactor * 0.3f,  // width (narrower)
        params.scaleFactor * 0.3f   // height (flatter)
    );
    
    // Head position (front of body)
    params.headCenter = DirectX::XMFLOAT3(
        params.bodyRadii.x * 0.8f, 
        0.0f, 
        0.0f
    );
    params.headRadius = 0.2f * params.scaleFactor;
    
    // More limbs for arthropods (6-10 typical)
    // Clamp to reasonable range
    params.limbCount = std::max(6, params.limbCount);
}

// Mollusca: Shell spiral parameters, mantle texture, movement pattern
// From tables.txt TABLE 7 (lines 462-474) and TABLE 12 (lines 657-660)
void GeneticMapper::ApplyMolluscaParams(CreatureParams& params, const Genetics::Genome& genome) const {
    // Body proportions for mollusks (rounded, compact)
    params.bodyCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    params.bodyRadii = DirectX::XMFLOAT3(
        params.scaleFactor * 0.8f,  // length
        params.scaleFactor * 0.8f,  // width
        params.scaleFactor * 0.6f   // height (shorter)
    );
    
    // Head position (top of body)
    params.headCenter = DirectX::XMFLOAT3(
        0.0f, 
        params.bodyRadii.y * 0.5f, 
        0.0f
    );
    params.headRadius = 0.25f * params.scaleFactor;
    
    // Tentacles instead of limbs (0-8)
    // Keep limbCount as tentacle count
}

} // namespace Generation
} // namespace Procedural
} // namespace Engine
