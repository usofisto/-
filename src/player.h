#pragma once
#include <string>
#include <vector>
#include "game_types.h"

// ============================================================
// ДАННЫЕ ИГРОКА
// ============================================================
struct Player {
    std::string name;
    std::string className;
    int health = 0;
    int maxHealth = 0;
    int damage = 0;
    int armor = 0;
    float speed = 190.0f;
    int gold = 100;
    bool isAlive = true;
    
    // Система инвентаря (стакуемая)
    std::vector<ItemType> inventory;
    
    // Экипировка
    ItemType equippedHelmet = ITEM_NONE;
    ItemType equippedChest = ITEM_NONE;
    ItemType equippedBoots = ITEM_NONE;
    ItemType equippedWeapon = ITEM_NONE;
    
    // Система опыта
    LevelSystem levelSystem;
    
    // Эффекты
    float speedBoostTimer = 0.0f;
    float strengthBoostTimer = 0.0f;
    int armorBonus = 0;
    
    // Получение урона с учётом брони
    int TakeDamage(int rawDamage) {
        int totalArmor = armor + armorBonus + levelSystem.bonusArmor;
        int actualDamage = std::max(1, rawDamage - totalArmor / 3);
        health = std::max(0, health - actualDamage);
        if (health <= 0) isAlive = false;
        return actualDamage;
    }
    
    // Получение лечения
    void Heal(int amount) {
        health = std::min(maxHealth, health + amount);
    }
    
    // Обновление таймеров эффектов
    void UpdateEffects(float dt) {
        if (speedBoostTimer > 0) {
            speedBoostTimer -= dt;
            speed = 250.0f;
        } else {
            speed = 190.0f + levelSystem.bonusSpeed;
        }
        
        if (strengthBoostTimer > 0) {
            strengthBoostTimer -= dt;
        }
    }
    
    // Получение бонусного урона
    int GetTotalDamage() {
        int baseDamage = damage + levelSystem.bonusDamage;
        if (strengthBoostTimer > 0) baseDamage = (int)(baseDamage * 1.5f);
        return baseDamage;
    }
    
    // Добавить предмет в инвентарь
    void AddItem(ItemType type, int count = 1) {
        for (int i = 0; i < count; i++) {
            inventory.push_back(type);
        }
    }
    
    // Подсчитать количество предметов
    int CountItems(ItemType type) {
        int count = 0;
        for (auto& item : inventory) {
            if (item == type) count++;
        }
        return count;
    }
    
    // Удалить предметы
    bool RemoveItems(ItemType type, int count) {
        if (CountItems(type) < count) return false;
        int removed = 0;
        for (auto it = inventory.begin(); it != inventory.end() && removed < count;) {
            if (*it == type) {
                it = inventory.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return true;
    }
};
