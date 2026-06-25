#pragma once
#include "raylib.h"
#include "game_types.h"

// Глобальный пул частиц
extern const int MAX_PARTICLES;
extern Particle particles[];

// Инициализация пула частиц при старте
void InitParticles();

// Функция для создания частицы
void SpawnParticle(Vector2 position, Vector2 velocity, Color color, float size, float lifetime);

// Обновление и отрисовка частиц в мире
void UpdateAndDrawParticles(float dt);
