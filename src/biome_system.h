#pragma once
#include "raylib.h"
#include <string>
#include <cmath>

enum BiomeType {
    BIOME_FOREST = 0,
    BIOME_DESERT,
    BIOME_SNOW,
    BIOME_SWAMP,
    BIOME_MEADOW,
    BIOME_COUNT
};

struct BiomeInfo {
    BiomeType type;
    const char* name;
    Color tintColor;      // Tint applied to all textures in this biome
    Color groundColor;    // Base ground color
    Color grassTint;      // Tint for grass textures
    float tintStrength;   // How strong the tint is (0-1)
};

// Biome definitions with color tints
inline const BiomeInfo BIOMES[] = {
    // Forest - green, natural
    {BIOME_FOREST, "Лес", 
     Color{100, 200, 120, 40},   // light green tint
     Color{34, 85, 44, 255},     // dark green ground
     Color{80, 180, 100, 50},    // grass tint
     0.3f},
    
    // Desert - sandy, warm
    {BIOME_DESERT, "Пустыня",
     Color{220, 180, 100, 60},   // sandy tint
     Color{180, 150, 90, 255},   // sandy ground
     Color{200, 170, 80, 70},    // dry grass tint
     0.4f},
    
    // Snow - cold, blue-white
    {BIOME_SNOW, "Снег",
     Color{200, 220, 240, 80},   // cold blue tint
     Color{210, 220, 230, 255},  // white ground
     Color{180, 200, 220, 90},   // snowy grass tint
     0.5f},
    
    // Swamp - dark, murky
    {BIOME_SWAMP, "Болото",
     Color{60, 80, 40, 70},      // dark murky tint
     Color{40, 55, 30, 255},     // dark ground
     Color{50, 70, 35, 80},      // murky grass tint
     0.4f},
    
    // Meadow - bright, colorful
    {BIOME_MEADOW, "Луга",
     Color{120, 200, 80, 30},    // bright green tint
     Color{60, 130, 50, 255},    // bright green ground
     Color{100, 190, 70, 40},    // fresh grass tint
     0.25f},
};

// Simple noise function for organic borders
inline float SimpleNoise(float x, float y) {
    float n = sinf(x * 12.9898f + y * 78.233f) * 43758.5453f;
    return n - floorf(n);
}

// Get biome at world position with smooth transitions
// World is 3000x3000
inline BiomeType GetBiomeAtPosition(float worldX, float worldY) {
    // Normalize to 0-1
    float nx = worldX / 3000.0f;
    float ny = worldY / 3000.0f;
    
    // Biome centers for Voronoi-like distribution
    struct Vec2 { float x, y; };
    const Vec2 centers[BIOME_COUNT] = {
        {0.2f, 0.2f},   // Forest - top-left
        {0.8f, 0.2f},   // Desert - top-right
        {0.8f, 0.8f},   // Snow - bottom-right
        {0.2f, 0.8f},   // Swamp - bottom-left
        {0.5f, 0.5f},   // Meadow - center
    };
    
    // Add noise to create organic borders
    float noiseX = SimpleNoise(nx * 3.0f, ny * 3.0f) * 0.15f;
    float noiseY = SimpleNoise(nx * 3.0f + 100.0f, ny * 3.0f + 100.0f) * 0.15f;
    
    float testX = nx + noiseX;
    float testY = ny + noiseY;
    
    // Find closest biome center
    float minDist = 1e9f;
    int closest = 0;
    
    for (int i = 0; i < BIOME_COUNT; i++) {
        float dx = testX - centers[i].x;
        float dy = testY - centers[i].y;
        float dist = sqrtf(dx*dx + dy*dy);
        
        if (dist < minDist) {
            minDist = dist;
            closest = i;
        }
    }
    
    return (BiomeType)closest;
}

// Get biome info
inline const BiomeInfo& GetBiomeInfo(BiomeType type) {
    return BIOMES[(int)type];
}

// Apply biome tint to a color
inline Color ApplyBiomeTint(Color original, Color tint, float strength) {
    Color result;
    result.r = (unsigned char)(original.r * (1.0f - strength) + tint.r * strength);
    result.g = (unsigned char)(original.g * (1.0f - strength) + tint.g * strength);
    result.b = (unsigned char)(original.b * (1.0f - strength) + tint.b * strength);
    result.a = original.a;
    return result;
}
