#pragma once
#include "raylib.h"
#include "game_types.h"
#include <string>

// Отрисовка персонажа
void DrawPlayerAvatar(const std::string& className, Vector2 position, float radius, Vector2 facing, bool isMoving, int framesCounter, Color bgDark);

// Отрисовка дерева
void DrawTree(Vector2 position, float radius);

// Отрисовка каменного бортика костра
void DrawFirePit(Vector2 position);

// Отрисовка костра
void DrawCampfire(Vector2 position, int framesCounter);

// Отрисовка палатки
void DrawTent(Vector2 position);

// Отрисовка озера
void DrawWaterPond(Vector2 position, float radius, int framesCounter);

// Отрисовка каменной стены руин
void DrawStoneWall(const StoneWall& wall);

// Отрисовка собираемого предмета
void DrawMapItem(const MapItem& item);

// Отрисовка слизня
void DrawWanderingSlime(const WanderingSlime& slime, int framesCounter, Color bgDark);

// Частицы
void SpawnParticle(Vector2 position, Vector2 velocity, Color color, float size, float lifetime);

// Вспомогательная функция отрисовки текстуры из центра
void DrawTextureCentered(Texture2D tex, Vector2 position, float scale, float rotation = 0.0f);
