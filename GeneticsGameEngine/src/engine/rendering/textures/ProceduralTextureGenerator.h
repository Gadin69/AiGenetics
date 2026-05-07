#pragma once

#include <DirectXMath.h>
#include <vector>
#include <string>
#include <functional>

namespace GeneticsGameEngine {
namespace Rendering {

// Procedural texture generator for biological materials
class ProceduralTextureGenerator
{
public:
    ProceduralTextureGenerator();
    ~ProceduralTextureGenerator();
    
    // Generate procedural textures
    struct TextureData {
        std::vector<uint8_t> pixels;  // RGBA pixels
        uint32_t width;
        uint32_t height;
    };
    
    // Generate skin texture with pores and subtle variation
    TextureData GenerateSkinTexture(uint32_t width, uint32_t height, 
                                     float scale = 1.0f, 
                                     DirectX::XMFLOAT3 baseColor = {0.95f, 0.75f, 0.70f});
    
    // Generate exoskeleton texture with segments and ridges
    TextureData GenerateExoskeletonTexture(uint32_t width, uint32_t height,
                                            float scale = 1.0f,
                                            DirectX::XMFLOAT3 baseColor = {0.3f, 0.2f, 0.15f});
    
    // Generate shell texture with spiral patterns
    TextureData GenerateShellTexture(uint32_t width, uint32_t height,
                                      float scale = 1.0f,
                                      DirectX::XMFLOAT3 baseColor = {0.85f, 0.80f, 0.70f});
    
    // Generate normal map from height data
    TextureData GenerateNormalMap(const TextureData& heightData, float strength = 1.0f);
    
    // Generate roughness/metallic/AO packed texture (RMA)
    TextureData GenerateRMATexture(uint32_t width, uint32_t height,
                                    float roughness = 0.5f,
                                    float metallic = 0.0f,
                                    float ao = 1.0f);
    
    // Noise functions
    float PerlinNoise2D(float x, float y) const;
    float FractionalBrownianMotion(float x, float y, int octaves, float persistence = 0.5f) const;
    
private:
    // Helper functions
    float SmoothNoise(float x, float y) const;
    float Interpolate(float a, float b, float t) const;
    uint8_t ClampFloatToByte(float value) const;
    
    // Seed for reproducible textures
    uint32_t seed_ = 42;
};

} // namespace Rendering
} // namespace GeneticsGameEngine
