#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <cmath>

// Игровые состояния
enum GameState {
    STATE_MENU,
    STATE_WELCOME,
    STATE_CLASS_SELECT,
    STATE_2D_WORLD,
    STATE_MEADOW_SLIME,
    STATE_GAME_OVER
};

// Типы предметов
enum ItemType {
    ITEM_NONE,
    ITEM_WOOD, ITEM_STONE, ITEM_COAL, ITEM_IRON, ITEM_GOLD, ITEM_DIAMOND,
    ITEM_WOOD_PLANK, ITEM_STICK, ITEM_TORCH,
    ITEM_WOOD_PICKAXE, ITEM_WOOD_AXE, ITEM_WOOD_SWORD,
    ITEM_STONE_PICKAXE, ITEM_STONE_AXE, ITEM_STONE_SWORD,
    ITEM_APPLE, ITEM_BREAD, ITEM_MEAT, ITEM_CAKE,
    ITEM_WHEAT, ITEM_SUGAR, ITEM_EGG,
    ITEM_HERB, ITEM_GOLDFLOWER, ITEM_MANA_CRYSTAL
};

struct MapItem { Vector2 position; ItemType type; bool active; float respawnTimer; };
struct StoneWall { Rectangle rect; };

struct TreeInfo {
    Vector2 position;
    float radius;
    int hp, maxHp;
    bool active;
    float shakeTimer;
    float respawnTimer;
    int treeType; // 1-10 for unique textures
};

struct RockInfo {
    Vector2 position;
    float radius;
    int hp, maxHp;
    bool active;
    float shakeTimer;
    float respawnTimer;
    int rockType; // 1-5 for unique textures
};

struct WanderingSlime {
    Vector2 position, targetPosition;
    float wanderTimer, speed;
    int hp, maxHp;
    bool active;
    int id;
    int slimeType; // 1=green, 2=red, 3=blue
};

struct FloatingText {
    std::string text;
    Vector2 position;
    Color color;
    float alpha, ySpeed, lifetime;
    bool active;
};

struct Particle {
    Vector2 position, velocity;
    Color color;
    float size, lifetime, maxLifetime;
    bool active;
};

struct Item {
    ItemType type;
    int amount;
    Item() : type(ITEM_NONE), amount(0) {}
    Item(ItemType t, int a) : type(t), amount(a) {}
};

struct SurvivalStats {
    int health, maxHealth, hunger, maxHunger;
    float hungerTimer;
    SurvivalStats() : health(20), maxHealth(20), hunger(20), maxHunger(20), hungerTimer(30.0f) {}
};

enum DayPhase { DAY_MORNING, DAY_AFTERNOON, DAY_EVENING, DAY_NIGHT };

struct DayNightCycle {
    float timeOfDay;
    DayPhase currentPhase;
    float daySpeed;

    DayNightCycle() : timeOfDay(8.0f), currentPhase(DAY_MORNING), daySpeed(0.5f) {}

    void Update(float dt) {
        timeOfDay += dt * daySpeed;
        if (timeOfDay >= 24.0f) timeOfDay -= 24.0f;
        if (timeOfDay >= 6.0f && timeOfDay < 12.0f) currentPhase = DAY_MORNING;
        else if (timeOfDay >= 12.0f && timeOfDay < 18.0f) currentPhase = DAY_AFTERNOON;
        else if (timeOfDay >= 18.0f && timeOfDay < 21.0f) currentPhase = DAY_EVENING;
        else currentPhase = DAY_NIGHT;
    }
};

struct CraftingRecipe {
    std::string name;
    std::vector<ItemType> ingredients;
    std::vector<int> amounts;
    ItemType result;
    int resultAmount;
};
