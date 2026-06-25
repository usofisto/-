#pragma once
#include <string>
#include <vector>
#include "game_types.h" // Подключаем типы предметов (ItemType)

// Данные игрока, все базовые характеристики и инвентарь
struct Player {
    std::string name;       // имя
    std::string className;  // класс (Воин, Маг, Лучник)
    int health = 0;         // текущие HP
    int maxHealth = 0;      // максимум HP
    int damage = 0;         // урон
    int gold = 100;         // золото
    bool isAlive = true;    // жив или нет
    
    std::vector<ItemType> inventory; // сумка с собранными предметами
};
