#pragma once

#include <DirectXMath.h>

namespace Engine {
namespace Procedural {
namespace Generation {

// Creature parameters mapped from genetic loci
// From tables.txt TABLE 12 (lines 633-644): Complete genetic loci mapping
struct CreatureParams {
    // Genetic parameters (from TABLE 12)
    float scaleFactor;         // Locus 0x1A2B, range 0.5-3.0
    int colorPaletteIndex;     // Locus 0x3C4D, range 0-7
    int limbCount;             // Locus 0x5E6F, range 1-8
    int voxelResolution;       // Locus 0x7G8H, values 32/64/128
    float surfaceSmoothness;   // Locus 0x9I0J, range 0-100
    float internalCavity;      // Locus 0x1K2L, range 0.0-1.0
    float roughness;           // Locus 0x3M4N, PBR parameter 0.0-1.0
    float metallic;            // Locus 0x5O6P, PBR parameter 0.0-1.0
    float subsurfaceScattering;// Locus 0x7Q8R, range 0.0-1.0
    
    // Body plan parameters (from tables.txt TABLE 7)
    DirectX::XMFLOAT3 bodyCenter;
    DirectX::XMFLOAT3 bodyRadii;
    DirectX::XMFLOAT3 headCenter;
    float headRadius;
    int taxonomyType; // 0=Chordata, 1=Arthropoda, 2=Mollusca
    
    CreatureParams() : 
        scaleFactor(1.0f), 
        colorPaletteIndex(0), 
        limbCount(4), 
        voxelResolution(64),
        surfaceSmoothness(50.0f), 
        internalCavity(0.0f),
        roughness(0.5f), 
        metallic(0.0f), 
        subsurfaceScattering(0.0f),
        bodyCenter{0.0f, 0.0f, 0.0f},
        bodyRadii{1.0f, 0.5f, 0.5f},
        headCenter{0.0f, 0.8f, 0.0f},
        headRadius(0.3f),
        taxonomyType(0) {}
};

} // namespace Generation
} // namespace Procedural
} // namespace Engine
