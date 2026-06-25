#pragma once
#include "raylib.h"
#include <string>

// Игровые состояния
enum GameState {
    STATE_WELCOME,         // Экран ввода имени (приветствие)
    STATE_CLASS_SELECT,    // Экран выбора класса
    STATE_2D_WORLD,        // Игра: свободное перемещение в 2D мире
    STATE_MEADOW_SLIME,    // Игра: сцена пошагового боя
    STATE_GAME_OVER        // Экран конца игры (гибель)
};

// Типы собираемых предметов (и инвентаря)
enum ItemType {
    ITEM_NONE,          // Пустой слот
    ITEM_HERB,          // Лечебная трава (можно применить для лечения)
    ITEM_GOLDFLOWER     // Золотоцвет (можно обменять или дает золото)
};

// Собираемый предмет на карте
struct MapItem {
    Vector2 position;
    ItemType type;
    bool active;
    float respawnTimer;
};

// Препятствие (дерево)
struct Obstacle {
    Vector2 position;
    float radius;
};

// Стена руин
struct StoneWall {
    Rectangle rect;
};

// Слизень на карте
struct WanderingSlime {
    Vector2 position;
    Vector2 targetPosition;
    float wanderTimer;
    float speed;
    int hp;
    int maxHp;
    bool active;
    int id; // уникальный идентификатор
};

// Всплывающий текст
struct FloatingText {
    std::string text;
    Vector2 position;
    Color color;
    float alpha;
    float ySpeed;
    float lifetime;
    bool active;
};

// Структура для эффектов частиц
struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float lifetime;
    float maxLifetime;
    bool active;
};
