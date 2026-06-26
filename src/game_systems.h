#pragma once
#include "game_types.h"
#include "player.h"
#include <vector>
#include <string>
#include <cmath>

// ============================================================
// СИСТЕМА ФЕРМЕРСТВА
// ============================================================
class FarmingSystem {
public:
    std::vector<FarmPlot> plots;
    
    // Добавить грядку
    void AddPlot(Vector2 pos) {
        FarmPlot plot;
        plot.position = pos;
        plot.hasSeed = false;
        plot.growStage = 0;
        plot.growTimer = 0;
        plot.isWatered = false;
        plot.waterTimer = 0;
        plots.push_back(plot);
    }
    
    // Посадить семя
    bool PlantSeed(Vector2 playerPos, CropType type, Player& player) {
        ItemType seedType;
        switch (type) {
            case CROP_WHEAT: seedType = ITEM_SEED_WHEAT; break;
            case CROP_CARROT: seedType = ITEM_SEED_CARROT; break;
            case CROP_POTATO: seedType = ITEM_SEED_POTATO; break;
            default: return false;
        }
        
        if (player.CountItems(seedType) <= 0) return false;
        
        for (auto& plot : plots) {
            float dx = playerPos.x - plot.position.x;
            float dy = playerPos.y - plot.position.y;
            if (sqrtf(dx*dx + dy*dy) < 40 && !plot.hasSeed) {
                player.RemoveItems(seedType, 1);
                plot.hasSeed = true;
                plot.cropType = type;
                plot.growStage = 0;
                plot.growTimer = 30.0f; // 30 секунд на стадию
                return true;
            }
        }
        return false;
    }
    
    // Полить грядку
    bool WaterPlot(Vector2 playerPos) {
        for (auto& plot : plots) {
            float dx = playerPos.x - plot.position.x;
            float dy = playerPos.y - plot.position.y;
            if (sqrtf(dx*dx + dy*dy) < 40 && plot.hasSeed) {
                plot.isWatered = true;
                plot.waterTimer = 60.0f;
                return true;
            }
        }
        return false;
    }
    
    // Собрать урожай
    bool Harvest(Vector2 playerPos, Player& player) {
        for (auto& plot : plots) {
            float dx = playerPos.x - plot.position.x;
            float dy = playerPos.y - plot.position.y;
            if (sqrtf(dx*dx + dy*dy) < 40 && plot.hasSeed && plot.growStage >= 3) {
                ItemType crop;
                int amount = 2 + (plot.isWatered ? 1 : 0);
                switch (plot.cropType) {
                    case CROP_WHEAT: crop = ITEM_WHEAT; break;
                    case CROP_CARROT: crop = ITEM_CARROT; break;
                    case CROP_POTATO: crop = ITEM_POTATO; break;
                    default: return false;
                }
                player.AddItem(crop, amount);
                plot.hasSeed = false;
                plot.growStage = 0;
                plot.isWatered = false;
                return true;
            }
        }
        return false;
    }
    
    // Обновление роста
    void Update(float dt) {
        for (auto& plot : plots) {
            if (!plot.hasSeed) continue;
            
            // Таймер полива
            if (plot.isWatered) {
                plot.waterTimer -= dt;
                if (plot.waterTimer <= 0) plot.isWatered = false;
            }
            
            // Рост (быстрее если полито)
            float growSpeed = plot.isWatered ? 1.5f : 1.0f;
            plot.growTimer -= dt * growSpeed;
            
            if (plot.growTimer <= 0 && plot.growStage < 3) {
                plot.growStage++;
                plot.growTimer = 30.0f;
            }
        }
    }
};

// ============================================================
// СИСТЕМА ПОГОДЫ
// ============================================================
class WeatherController {
public:
    WeatherSystem weather;
    
    void Update(float dt, int biomeType, float& speedMod) {
        weather.Update(dt, biomeType);
        
        // Модификатор скорости от погоды
        switch (weather.current) {
            case WEATHER_RAIN: speedMod = 0.85f; break;
            case WEATHER_SNOW: speedMod = 0.7f; break;
            case WEATHER_FOG: speedMod = 0.95f; break;
            default: speedMod = 1.0f; break;
        }
    }
    
    // Получить цвет частиц погоды
    Color GetWeatherParticleColor() {
        switch (weather.current) {
            case WEATHER_RAIN: return Color{100, 150, 255, 150};
            case WEATHER_SNOW: return Color{240, 245, 255, 200};
            case WEATHER_FOG: return Color{200, 200, 200, 80};
            default: return Color{0, 0, 0, 0};
        }
    }
};

// ============================================================
// СИСТЕМА КВЕСТОВ
// ============================================================
class QuestController {
public:
    QuestSystem quests;
    
    void GenerateDailyQuests(int day) {
        quests.dailyQuests.clear();
        
        Quest q;
        
        // Квест на убийство слизней
        q.type = QUEST_KILL_SLIMES;
        q.title = "Охота на слизней";
        q.description = "Убей 5 слизней";
        q.target = 5;
        q.progress = 0;
        q.xpReward = 50 + day * 10;
        q.goldReward = 20 + day * 5;
        q.completed = false;
        q.active = true;
        quests.dailyQuests.push_back(q);
        
        // Квест на добычу дерева
        q.type = QUEST_GATHER_WOOD;
        q.title = "Лесоруб";
        q.description = "Сруби 10 деревьев";
        q.target = 10;
        q.progress = 0;
        q.xpReward = 40 + day * 8;
        q.goldReward = 15 + day * 3;
        q.completed = false;
        q.active = true;
        quests.dailyQuests.push_back(q);
        
        // Квест на добычу камня
        q.type = QUEST_GATHER_STONE;
        q.title = "Шахтёр";
        q.description = "Собери 8 камней";
        q.target = 8;
        q.progress = 0;
        q.xpReward = 45 + day * 9;
        q.goldReward = 18 + day * 4;
        q.completed = false;
        q.active = true;
        quests.dailyQuests.push_back(q);
        
        // Квест на рыбалку
        if (day >= 2) {
            q.type = QUEST_FISH;
            q.title = "Рыболов";
            q.description = "Поймай 3 рыбы";
            q.target = 3;
            q.progress = 0;
            q.xpReward = 60 + day * 12;
            q.goldReward = 25 + day * 5;
            q.completed = false;
            q.active = true;
            quests.dailyQuests.push_back(q);
        }
        
        // Квест на волков
        if (day >= 3) {
            q.type = QUEST_KILL_WOLF;
            q.title = "Волчья охота";
            q.description = "Убей 3 волков";
            q.target = 3;
            q.progress = 0;
            q.xpReward = 80 + day * 15;
            q.goldReward = 35 + day * 7;
            q.completed = false;
            q.active = true;
            quests.dailyQuests.push_back(q);
        }
    }
    
    // Обновить прогресс квеста
    void UpdateQuest(QuestType type, int amount = 1) {
        for (auto& q : quests.dailyQuests) {
            if (q.active && !q.completed && q.type == type) {
                q.progress += amount;
                if (q.progress >= q.target) {
                    q.completed = true;
                    quests.questsCompleted++;
                }
            }
        }
    }
    
    // Получить награду за квест
    bool ClaimReward(int index, Player& player) {
        if (index < 0 || index >= (int)quests.dailyQuests.size()) return false;
        auto& q = quests.dailyQuests[index];
        if (!q.completed) return false;
        
        player.levelSystem.AddXP(q.xpReward);
        player.gold += q.goldReward;
        q.active = false;
        return true;
    }
};

// ============================================================
// СИСТЕМА ТОРГОВЦА
// ============================================================
class TradingSystem {
public:
    std::vector<TradeOffer> offers;
    Merchant merchant;
    
    void InitMerchant(Vector2 pos) {
        merchant.position = pos;
        merchant.name = "Торговец Гильд";
        merchant.active = true;
        merchant.interactRadius = 60.0f;
        
        // Торговые предложения
        offers.clear();
        TradeOffer offer;
        
        // Покупка ресурсов
        offer.give = ITEM_GOLD; offer.giveAmount = 10;
        offer.receive = ITEM_WOOD; offer.receiveAmount = 5;
        offers.push_back(offer);
        
        offer.give = ITEM_GOLD; offer.giveAmount = 15;
        offer.receive = ITEM_STONE; offer.receiveAmount = 5;
        offers.push_back(offer);
        
        offer.give = ITEM_GOLD; offer.giveAmount = 30;
        offer.receive = ITEM_IRON; offer.receiveAmount = 3;
        offers.push_back(offer);
        
        offer.give = ITEM_GOLD; offer.giveAmount = 50;
        offer.receive = ITEM_DIAMOND; offer.receiveAmount = 1;
        offers.push_back(offer);
        
        // Продажа ресурсов
        offer.give = ITEM_WOOD; offer.giveAmount = 10;
        offer.receive = ITEM_GOLD; offer.receiveAmount = 15;
        offers.push_back(offer);
        
        offer.give = ITEM_STONE; offer.giveAmount = 10;
        offer.receive = ITEM_GOLD; offer.receiveAmount = 20;
        offers.push_back(offer);
        
        // Зелья
        offer.give = ITEM_GOLD; offer.giveAmount = 40;
        offer.receive = ITEM_POTION_HP; offer.receiveAmount = 1;
        offers.push_back(offer);
        
        offer.give = ITEM_GOLD; offer.giveAmount = 60;
        offer.receive = ITEM_POTION_SPEED; offer.receiveAmount = 1;
        offers.push_back(offer);
    }
    
    bool CanTrade(int offerIndex, Player& player) {
        if (offerIndex < 0 || offerIndex >= (int)offers.size()) return false;
        auto& offer = offers[offerIndex];
        return player.CountItems(offer.give) >= offer.giveAmount;
    }
    
    bool Trade(int offerIndex, Player& player) {
        if (!CanTrade(offerIndex, player)) return false;
        auto& offer = offers[offerIndex];
        player.RemoveItems(offer.give, offer.giveAmount);
        player.AddItem(offer.receive, offer.receiveAmount);
        return true;
    }
    
    bool IsNearMerchant(Vector2 playerPos) {
        float dx = playerPos.x - merchant.position.x;
        float dy = playerPos.y - merchant.position.y;
        return sqrtf(dx*dx + dy*dy) < merchant.interactRadius;
    }
};

// ============================================================
// СИСТЕМА РЫБАЛКИ
// ============================================================
class FishingController {
public:
    std::vector<FishingSpot> spots;
    bool isFishing = false;
    float fishTimer = 0;
    float catchWindow = 0;
    bool fishBiting = false;
    
    void AddSpot(Vector2 pos) {
        FishingSpot spot;
        spot.position = pos;
        spot.active = true;
        spot.fishTimer = 10.0f + (rand() % 20);
        spot.fishCount = 3 + rand() % 5;
        spots.push_back(spot);
    }
    
    bool StartFishing(Vector2 playerPos) {
        for (auto& spot : spots) {
            float dx = playerPos.x - spot.position.x;
            float dy = playerPos.y - spot.position.y;
            if (sqrtf(dx*dx + dy*dy) < 50 && spot.active && spot.fishCount > 0) {
                isFishing = true;
                fishTimer = 3.0f + (rand() % 4);
                fishBiting = false;
                return true;
            }
        }
        return false;
    }
    
    void Update(float dt) {
        if (!isFishing) return;
        
        fishTimer -= dt;
        if (fishTimer <= 0 && !fishBiting) {
            fishBiting = true;
            catchWindow = 1.5f; // 1.5 секунды на подсечку
        }
        
        if (fishBiting) {
            catchWindow -= dt;
            if (catchWindow <= 0) {
                // Рыба ушла
                isFishing = false;
                fishBiting = false;
            }
        }
    }
    
    bool TryCatch() {
        if (isFishing && fishBiting) {
            isFishing = false;
            fishBiting = false;
            return true;
        }
        return false;
    }
};

// ============================================================
// СИСТЕМА УРОВНЕЙ
// ============================================================
inline void ApplyLevelBonuses(Player& player) {
    auto& ls = player.levelSystem;
    // Каждые 2 очка навыка можно распределить
    // Пока автоматически: +1 HP, +1 DMG каждые 2 уровня
    player.maxHealth = 20 + ls.bonusHP + (ls.level / 2);
    player.damage = 10 + ls.bonusDamage + (ls.level / 3);
}
