#pragma once
#include <string>
#include <vector>
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
    std::string name;
    std::string grassPrefix;
    std::string treePrefix;
    int grassCount;
    int treeCount;
    unsigned char baseR, baseG, baseB; // Base ground color
};

// Biome definitions
inline const BiomeInfo BIOMES[] = {
    {BIOME_FOREST, "Лес",    "forest/grass_",  "forest/tree_",  5, 3, 34, 85, 44},
    {BIOME_DESERT, "Пустыня","desert/grass_",   "desert/tree_",  5, 3, 180, 150, 90},
    {BIOME_SNOW,   "Снег",   "snow/grass_",     "snow/tree_",    5, 3, 200, 210, 220},
    {BIOME_SWAMP,  "Болото", "swamp/grass_",    "swamp/tree_",   5, 3, 50, 65, 35},
    {BIOME_MEADOW, "Луга",   "meadow/grass_",   "meadow/tree_",  5, 3, 60, 130, 50},
};

// Get biome at world position using a more organic distribution
// World is 2000x2000, divided into 5 regions with soft borders
inline BiomeType GetBiomeAtPosition(float worldX, float worldY) {
    // Normalize to 0-1
    float nx = worldX / 2000.0f;
    float ny = worldY / 2000.0f;
    
    // Create biome zones using distance from key points
    // Each biome has a "center" and we pick the closest one
    
    // Biome centers (x, y)
    struct Vec2 { float x, y; };
    const Vec2 centers[BIOME_COUNT] = {
        {0.25f, 0.25f},  // Forest - top-left
        {0.75f, 0.25f},  // Desert - top-right
        {0.75f, 0.75f},  // Snow - bottom-right
        {0.25f, 0.75f},  // Swamp - bottom-left
        {0.50f, 0.50f},  // Meadow - center
    };
    
    // Find closest biome center
    float minDist = 1e9f;
    int closest = 0;
    
    for (int i = 0; i < BIOME_COUNT; i++) {
        float dx = nx - centers[i].x;
        float dy = ny - centers[i].y;
        float dist = sqrtf(dx*dx + dy*dy);
        
        // Add some noise-like variation based on position
        // This creates more organic borders
        float noise = sinf(nx * 6.28f + i * 1.5f) * cosf(ny * 6.28f + i * 2.3f) * 0.08f;
        dist += noise;
        
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
