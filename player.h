#pragma once
#include <string>

// СТРУКТУРА ИГРОКА (Player)
// Хранит все базовые характеристики персонажа
struct Player {
    std::string name;       // Имя игрока
    std::string className;  // Выбранный класс (Воин, Маг, Лучник)
    int health;             // Текущее здоровье (HP)
    int maxHealth;          // Максимальный запас здоровья
    int damage;             // Сила атаки (урон)
    int gold;               // Золотые монеты
    bool isAlive = true;    // Статус жизни персонажа (true = жив, false = мертв)
};
