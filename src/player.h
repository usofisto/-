#pragma once
#include <string>

// СТРУКТУРА ИГРОКА (Player)
// Хранит все базовые характеристики персонажа
struct Player {
    std::string name;       // Имя игрока
    std::string className;  // Выбранный класс (Воин, Маг, Лучник)
    int health = 0;         // Текущее здоровье (HP)
    int maxHealth = 0;      // Максимальный запас здоровья
    int damage = 0;         // Сила атаки (урон)
    int gold = 100;         // Золотые монеты
    bool isAlive = true;    // Статус жизни персонажа (true = жив, false = мертв)
};
