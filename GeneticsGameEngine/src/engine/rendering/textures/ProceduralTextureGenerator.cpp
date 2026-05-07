#include "ProceduralTextureGenerator.h"
#include <cmath>
#include <algorithm>
#include <random>

using namespace GeneticsGameEngine::Rendering;
using namespace DirectX;

ProceduralTextureGenerator::ProceduralTextureGenerator()
{
}

ProceduralTextureGenerator::~ProceduralTextureGenerator()
{
}

// ============================================================================
// Noise Functions
// ============================================================================

float ProceduralTextureGenerator::SmoothNoise(float x, float y) const
{
    // Simple hash-based noise
    float n = sinf(x * 12.9898f + y * 78.233f + seed_) * 43758.5453f;
    return n - floorf(n);
}

float ProceduralTextureGenerator::Interpolate(float a, float b, float t) const
{
    // Smooth interpolation
    float t2 = t * t;
    float t3 = t2 * t;
    return a * (1.0f - 3.0f * t2 + 2.0f * t3) + b * (3.0f * t2 - 2.0f * t3);
}

float ProceduralTextureGenerator::PerlinNoise2D(float x, float y) const
{
    // Get grid coordinates
    int x0 = (int)floorf(x);
    int x1 = x0 + 1;
    int y0 = (int)floorf(y);
    int y1 = y0 + 1;
    
    // Get fractional parts
    float sx = x - (float)x0;
    float sy = y - (float)y0;
    
    // Get noise values at corners
    float n00 = SmoothNoise((float)x0, (float)y0);
    float n10 = SmoothNoise((float)x1, (float)y0);
    float n01 = SmoothNoise((float)x0, (float)y1);
    float n11 = SmoothNoise((float)x1, (float)y1);
    
    // Interpolate
    float nx0 = Interpolate(n00, n10, sx);
    float nx1 = Interpolate(n01, n11, sx);
    
    return Interpolate(nx0, nx1, sy);
}

float ProceduralTextureGenerator::FractionalBrownianMotion(float x, float y, int octaves, float persistence) const
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++)
    {
        total += PerlinNoise2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

uint8_t ProceduralTextureGenerator::ClampFloatToByte(float value) const
{
    return (uint8_t)std::max(0.0f, std::min(255.0f, value * 255.0f));
}

// ============================================================================
// Texture Generation
// ============================================================================

ProceduralTextureGenerator::TextureData ProceduralTextureGenerator::GenerateSkinTexture(
    uint32_t width, uint32_t height, float scale, XMFLOAT3 baseColor)
{
    TextureData texture;
    texture.width = width;
    texture.height = height;
    texture.pixels.resize(width * height * 4);
    
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            float nx = (float)x / (float)width * scale * 10.0f;
            float ny = (float)y / (float)height * scale * 10.0f;
            
            // Multi-layer noise for skin detail
            float noise1 = FractionalBrownianMotion(nx, ny, 4, 0.5f) * 0.1f;
            float noise2 = FractionalBrownianMotion(nx * 2.0f, ny * 2.0f, 6, 0.4f) * 0.05f;
            
            // Pores (small dark spots)
            float pore = PerlinNoise2D(nx * 5.0f, ny * 5.0f);
            pore = powf(std::max(0.0f, pore - 0.6f), 2.0f) * 0.3f;
            
            // Combine
            float variation = noise1 + noise2 - pore;
            
            float r = std::max(0.0f, std::min(1.0f, baseColor.x + variation));
            float g = std::max(0.0f, std::min(1.0f, baseColor.y + variation));
            float b = std::max(0.0f, std::min(1.0f, baseColor.z + variation * 0.8f));
            
            uint32_t idx = (y * width + x) * 4;
            texture.pixels[idx + 0] = ClampFloatToByte(r);
            texture.pixels[idx + 1] = ClampFloatToByte(g);
            texture.pixels[idx + 2] = ClampFloatToByte(b);
            texture.pixels[idx + 3] = 255; // Alpha
        }
    }
    
    return texture;
}

ProceduralTextureGenerator::TextureData ProceduralTextureGenerator::GenerateExoskeletonTexture(
    uint32_t width, uint32_t height, float scale, XMFLOAT3 baseColor)
{
    TextureData texture;
    texture.width = width;
    texture.height = height;
    texture.pixels.resize(width * height * 4);
    
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            float nx = (float)x / (float)width * scale * 8.0f;
            float ny = (float)y / (float)height * scale * 8.0f;
            
            // Segmentation lines (horizontal ridges)
            float segment = sinf(ny * 3.14159f * 4.0f);
            segment = powf(std::max(0.0f, segment), 8.0f) * 0.3f;
            
            // Surface roughness
            float noise = FractionalBrownianMotion(nx, ny, 3, 0.6f) * 0.15f;
            
            // Fine detail
            float detail = PerlinNoise2D(nx * 3.0f, ny * 3.0f) * 0.05f;
            
            float variation = noise + detail - segment;
            
            float r = std::max(0.0f, std::min(1.0f, baseColor.x + variation));
            float g = std::max(0.0f, std::min(1.0f, baseColor.y + variation * 0.9f));
            float b = std::max(0.0f, std::min(1.0f, baseColor.z + variation * 0.7f));
            
            uint32_t idx = (y * width + x) * 4;
            texture.pixels[idx + 0] = ClampFloatToByte(r);
            texture.pixels[idx + 1] = ClampFloatToByte(g);
            texture.pixels[idx + 2] = ClampFloatToByte(b);
            texture.pixels[idx + 3] = 255;
        }
    }
    
    return texture;
}

ProceduralTextureGenerator::TextureData ProceduralTextureGenerator::GenerateShellTexture(
    uint32_t width, uint32_t height, float scale, XMFLOAT3 baseColor)
{
    TextureData texture;
    texture.width = width;
    texture.height = height;
    texture.pixels.resize(width * height * 4);
    
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            float nx = (float)x / (float)width * scale * 6.0f;
            float ny = (float)y / (float)height * scale * 6.0f;
            
            // Spiral pattern
            float angle = atan2f(ny - 5.0f, nx - 5.0f);
            float radius = sqrtf((nx - 5.0f) * (nx - 5.0f) + (ny - 5.0f) * (ny - 5.0f));
            float spiral = sinf(angle * 3.0f + radius * 2.0f) * 0.5f + 0.5f;
            spiral = powf(spiral, 3.0f) * 0.2f;
            
            // Growth rings
            float rings = sinf(radius * 3.14159f * 2.0f) * 0.5f + 0.5f;
            rings = powf(rings, 4.0f) * 0.15f;
            
            // Surface noise
            float noise = FractionalBrownianMotion(nx, ny, 4, 0.5f) * 0.1f;
            
            float variation = noise + spiral + rings;
            
            float r = std::max(0.0f, std::min(1.0f, baseColor.x + variation));
            float g = std::max(0.0f, std::min(1.0f, baseColor.y + variation * 0.95f));
            float b = std::max(0.0f, std::min(1.0f, baseColor.z + variation * 0.9f));
            
            uint32_t idx = (y * width + x) * 4;
            texture.pixels[idx + 0] = ClampFloatToByte(r);
            texture.pixels[idx + 1] = ClampFloatToByte(g);
            texture.pixels[idx + 2] = ClampFloatToByte(b);
            texture.pixels[idx + 3] = 255;
        }
    }
    
    return texture;
}

ProceduralTextureGenerator::TextureData ProceduralTextureGenerator::GenerateNormalMap(
    const TextureData& heightData, float strength)
{
    TextureData normalMap;
    normalMap.width = heightData.width;
    normalMap.height = heightData.height;
    normalMap.pixels.resize(heightData.width * heightData.height * 4);
    
    uint32_t w = heightData.width;
    uint32_t h = heightData.height;
    
    for (uint32_t y = 0; y < h; y++)
    {
        for (uint32_t x = 0; x < w; x++)
        {
            // Sample height neighbors
            float hL = (float)heightData.pixels[((y * w + (x > 0 ? x - 1 : x)) * 4)] / 255.0f;
            float hR = (float)heightData.pixels[((y * w + (x < w - 1 ? x + 1 : x)) * 4)] / 255.0f;
            float hU = (float)heightData.pixels[(((y > 0 ? y - 1 : y) * w + x) * 4)] / 255.0f;
            float hD = (float)heightData.pixels[(((y < h - 1 ? y + 1 : y) * w + x) * 4)] / 255.0f;
            
            // Calculate normal
            float dx = (hR - hL) * strength;
            float dy = (hD - hU) * strength;
            
            // Normalize
            float len = sqrtf(dx * dx + dy * dy + 1.0f);
            float nx = -dx / len;
            float ny = -dy / len;
            float nz = 1.0f / len;
            
            // Convert to [0, 255] range
            uint32_t idx = (y * w + x) * 4;
            normalMap.pixels[idx + 0] = (uint8_t)((nx * 0.5f + 0.5f) * 255.0f);
            normalMap.pixels[idx + 1] = (uint8_t)((ny * 0.5f + 0.5f) * 255.0f);
            normalMap.pixels[idx + 2] = (uint8_t)((nz * 0.5f + 0.5f) * 255.0f);
            normalMap.pixels[idx + 3] = 255;
        }
    }
    
    return normalMap;
}

ProceduralTextureGenerator::TextureData ProceduralTextureGenerator::GenerateRMATexture(
    uint32_t width, uint32_t height, float roughness, float metallic, float ao)
{
    TextureData texture;
    texture.width = width;
    texture.height = height;
    texture.pixels.resize(width * height * 4);
    
    // Add subtle variation
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            float nx = (float)x / (float)width * 10.0f;
            float ny = (float)y / (float)height * 10.0f;
            
            float noise = FractionalBrownianMotion(nx, ny, 2, 0.5f) * 0.05f;
            
            float r = std::max(0.0f, std::min(1.0f, roughness + noise));
            float g = std::max(0.0f, std::min(1.0f, metallic + noise * 0.5f));
            float b = std::max(0.0f, std::min(1.0f, ao + noise * 0.3f));
            
            uint32_t idx = (y * width + x) * 4;
            texture.pixels[idx + 0] = ClampFloatToByte(r);
            texture.pixels[idx + 1] = ClampFloatToByte(g);
            texture.pixels[idx + 2] = ClampFloatToByte(b);
            texture.pixels[idx + 3] = 255;
        }
    }
    
    return texture;
}
