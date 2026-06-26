#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <cmath>

// ============================================================
// СОСТОЯНИЯ ИГРЫ
// ============================================================
enum GameState {
    STATE_MENU,
    STATE_WELCOME,
    STATE_CLASS_SELECT,
    STATE_2D_WORLD,
    STATE_MEADOW_SLIME,
    STATE_GAME_OVER,
    STATE_TRADING,       // Торговля с NPC
    STATE_FISHING,       // Рыбалка
    STATE_BOSS_FIGHT     // Бой с боссом
};

// ============================================================
// ТИПЫ ПРЕДМЕТОВ
// ============================================================
enum ItemType {
    ITEM_NONE = 0,
    // Ресурсы
    ITEM_WOOD, ITEM_STONE, ITEM_COAL, ITEM_IRON, ITEM_GOLD, ITEM_DIAMOND,
    ITEM_WOOD_PLANK, ITEM_STICK, ITEM_TORCH,
    // Инструменты
    ITEM_WOOD_PICKAXE, ITEM_WOOD_AXE, ITEM_WOOD_SWORD,
    ITEM_STONE_PICKAXE, ITEM_STONE_AXE, ITEM_STONE_SWORD,
    ITEM_IRON_PICKAXE, ITEM_IRON_AXE, ITEM_IRON_SWORD,
    // Еда
    ITEM_APPLE, ITEM_BREAD, ITEM_MEAT, ITEM_CAKE, ITEM_COOKED_MEAT, ITEM_SOUP,
    ITEM_EGG, ITEM_SUGAR,
    // Зелья
    ITEM_POTION_HP, ITEM_POTION_SPEED, ITEM_POTION_STRENGTH,
    // Броня
    ITEM_LEATHER_HELMET, ITEM_LEATHER_CHEST, ITEM_LEATHER_BOOTS,
    ITEM_IRON_HELMET, ITEM_IRON_CHEST, ITEM_IRON_BOOTS,
    // Фермерство
    ITEM_SEED_WHEAT, ITEM_SEED_CARROT, ITEM_SEED_POTATO,
    ITEM_WHEAT, ITEM_CARROT, ITEM_POTATO,
    // Рыбалка
    ITEM_FISH, ITEM_COOKED_FISH,
    // Прочее
    ITEM_HERB, ITEM_GOLDFLOWER, ITEM_MANA_CRYSTAL,
    ITEM_LEATHER, ITEM_BONE, ITEM_STRING,
    // Строительство
    ITEM_WALL_WOOD, ITEM_WALL_STONE, ITEM_FLOOR_WOOD,
    ITEM_CHEST, ITEM_BED, ITEM_FURNACE,
    // Босс дроп
    ITEM_BOSS_CORE, ITEM_LEGENDARY_SWORD,
    ITEM_COUNT
};

// ============================================================
// СТРУКТУРЫ
// ============================================================

struct MapItem { 
    Vector2 position; 
    ItemType type; 
    bool active; 
    float respawnTimer; 
};

struct StoneWall { 
    Rectangle rect; 
    bool isDestructible = false;
    int hp = 5;
};

struct TreeInfo {
    Vector2 position;
    float radius;
    int hp, maxHp;
    bool active;
    float shakeTimer;
    float respawnTimer;
    int treeType;
    int biomeType;
};

struct RockInfo {
    Vector2 position;
    float radius;
    int hp, maxHp;
    bool active;
    float shakeTimer;
    float respawnTimer;
    int rockType;
    int biomeType;
};

struct WanderingSlime {
    Vector2 position, targetPosition;
    float wanderTimer, speed;
    int hp, maxHp;
    bool active;
    int id;
    int slimeType; // 1=green, 2=red, 3=blue
};

// Новые враги
struct Wolf {
    Vector2 position, targetPosition;
    float wanderTimer, speed, attackCooldown;
    int hp, maxHp, damage;
    bool active, isChasing;
    int id;
};

struct Skeleton {
    Vector2 position, targetPosition;
    float wanderTimer, speed, attackCooldown;
    int hp, maxHp, damage;
    bool active;
    int id;
    bool isNightOnly = true;
};

struct Boss {
    Vector2 position;
    float attackCooldown, specialAttackTimer;
    int hp, maxHp, damage;
    bool active;
    int bossType; // 1=Forest Guardian, 2=Desert Worm, 3=Ice Golem
    std::string name;
    float radius;
};

// ============================================================
// ФЕРМЕРСТВО
// ============================================================
enum CropType { CROP_WHEAT, CROP_CARROT, CROP_POTATO };

struct FarmPlot {
    Vector2 position;
    CropType cropType;
    float growTimer;      // Время до следующей стадии
    int growStage;        // 0=семя, 1=росток, 2=взрослое, 3=готово
    bool hasSeed;
    bool isWatered;
    float waterTimer;     // Как долго полито
};

// ============================================================
// СТРОИТЕЛЬСТВО
// ============================================================
struct PlacedObject {
    Vector2 position;
    ItemType type;
    int rotation; // 0, 90, 180, 270
};

// ============================================================
// NPC И ТОРГОВЛЯ
// ============================================================
struct Merchant {
    Vector2 position;
    std::string name;
    bool active;
    float interactRadius;
};

struct TradeOffer {
    ItemType give;
    int giveAmount;
    ItemType receive;
    int receiveAmount;
};

// ============================================================
// РЫБАЛКА
// ============================================================
struct FishingSpot {
    Vector2 position;
    bool active;
    float fishTimer;
    int fishCount;
};

// ============================================================
// ЭФФЕКТЫ И ТЕКСТЫ
// ============================================================
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

// ============================================================
// ПРЕДМЕТ
// ============================================================
struct Item {
    ItemType type;
    int amount;
    Item() : type(ITEM_NONE), amount(0) {}
    Item(ItemType t, int a) : type(t), amount(a) {}
};

// ============================================================
// ВЫЖИВАНИЕ
// ============================================================
struct SurvivalStats {
    int health, maxHealth;
    int hunger, maxHunger;
    int thirst, maxThirst;
    float hungerTimer;
    float thirstTimer;
    
    SurvivalStats() : 
        health(20), maxHealth(20), 
        hunger(20), maxHunger(20),
        thirst(20), maxThirst(20),
        hungerTimer(30.0f), thirstTimer(20.0f) {}
};

// ============================================================
// ДЕНЬ/НОЧЬ
// ============================================================
enum DayPhase { DAY_MORNING, DAY_AFTERNOON, DAY_EVENING, DAY_NIGHT };

struct DayNightCycle {
    float timeOfDay;
    DayPhase currentPhase;
    float daySpeed;
    int dayCount;

    DayNightCycle() : timeOfDay(8.0f), currentPhase(DAY_MORNING), daySpeed(0.5f), dayCount(1) {}

    void Update(float dt) {
        timeOfDay += dt * daySpeed;
        if (timeOfDay >= 24.0f) {
            timeOfDay -= 24.0f;
            dayCount++;
        }
        if (timeOfDay >= 6.0f && timeOfDay < 12.0f) currentPhase = DAY_MORNING;
        else if (timeOfDay >= 12.0f && timeOfDay < 18.0f) currentPhase = DAY_AFTERNOON;
        else if (timeOfDay >= 18.0f && timeOfDay < 21.0f) currentPhase = DAY_EVENING;
        else currentPhase = DAY_NIGHT;
    }
};

// ============================================================
// ПОГОДА
// ============================================================
enum WeatherType { WEATHER_CLEAR, WEATHER_RAIN, WEATHER_SNOW, WEATHER_FOG };

struct WeatherSystem {
    WeatherType current;
    float weatherTimer;
    float intensity; // 0-1
    
    WeatherSystem() : current(WEATHER_CLEAR), weatherTimer(120.0f), intensity(0.0f) {}
    
    void Update(float dt, int biomeType) {
        weatherTimer -= dt;
        if (weatherTimer <= 0) {
            // Смена погоды
            int roll = rand() % 100;
            if (roll < 40) current = WEATHER_CLEAR;
            else if (roll < 65) current = (biomeType == 2) ? WEATHER_SNOW : WEATHER_RAIN;
            else if (roll < 80) current = WEATHER_FOG;
            else current = WEATHER_CLEAR;
            
            intensity = 0.3f + (rand() % 70) / 100.0f;
            weatherTimer = 60.0f + (rand() % 120);
        }
    }
};

// ============================================================
// ОПЫТ И УРОВНИ
// ============================================================
struct LevelSystem {
    int level;
    int xp;
    int xpToNext;
    int skillPoints;
    
    // Характеристики
    int bonusHP;
    int bonusDamage;
    int bonusSpeed;
    int bonusArmor;
    
    LevelSystem() : level(1), xp(0), xpToNext(100), skillPoints(0),
                    bonusHP(0), bonusDamage(0), bonusSpeed(0), bonusArmor(0) {}
    
    void AddXP(int amount) {
        xp += amount;
        while (xp >= xpToNext) {
            xp -= xpToNext;
            level++;
            xpToNext = (int)(xpToNext * 1.5f);
            skillPoints += 2;
        }
    }
};

// ============================================================
// КВЕСТЫ
// ============================================================
enum QuestType { 
    QUEST_KILL_SLIMES, QUEST_GATHER_WOOD, QUEST_GATHER_STONE,
    QUEST_BUILD_WALL, QUEST_FISH, QUEST_COOK, QUEST_FARM,
    QUEST_KILL_WOLF, QUEST_KILL_SKELETON, QUEST_KILL_BOSS
};

struct Quest {
    QuestType type;
    std::string title;
    std::string description;
    int target;
    int progress;
    int xpReward;
    int goldReward;
    bool completed;
    bool active;
    
    Quest() : type(QUEST_KILL_SLIMES), target(0), progress(0), 
              xpReward(0), goldReward(0), completed(false), active(false) {}
};

struct QuestSystem {
    std::vector<Quest> dailyQuests;
    float questRefreshTimer;
    int questsCompleted;
    
    QuestSystem() : questRefreshTimer(300.0f), questsCompleted(0) {}
};

// ============================================================
// КРАФТ РЕЦЕПТ
// ============================================================
struct CraftingRecipe {
    std::string name;
    std::vector<ItemType> ingredients;
    std::vector<int> amounts;
    ItemType result;
    int resultAmount;
};
