// ============================================================
// Красивый HUD с барами и мини-картой
// ============================================================
#pragma once
#include "raylib.h"
#include <string>
#include <sstream>
#include <cmath>

// Рисование красивого бара с градиентом и скруглениями
inline void DrawFancyBar(float x, float y, float w, float h, float value, float maxValue, Color fgColor, Color bgColor, const char* label, Font& font) {
    // Фон бара с тенью
    DrawRectangleRounded({x + 2, y + 2, w, h}, 0.3f, 8, Color{0, 0, 0, 80});
    DrawRectangleRounded({x, y, w, h}, 0.3f, 8, bgColor);
    DrawRectangleRoundedLines({x, y, w, h}, 0.3f, 8, 1.5f, Color{255, 255, 255, 30});
    
    // Значение бара
    float ratio = value / maxValue;
    if (ratio > 0) {
        float barW = w * ratio;
        // Градиентный эффект
        Color topColor = fgColor;
        Color botColor = {static_cast<unsigned char>(fgColor.r * 0.7f), static_cast<unsigned char>(fgColor.g * 0.7f), static_cast<unsigned char>(fgColor.b * 0.7f), fgColor.a};
        
        // Скруглённый бар
        DrawRectangleRounded({x, y, barW, h}, 0.3f, 8, topColor);
        
        // Блик сверху
        DrawRectangleRounded({x + 2, y + 1, barW - 4, h * 0.4f}, 0.3f, 8, Color{255, 255, 255, 40});
    }
    
    // Текст
    if (label) {
        Vector2 ts = MeasureTextEx(font, label, 11, 1);
        DrawTextEx(font, label, {x + 5, y + (h - ts.y) / 2}, 11, 1, Color{255, 255, 255, 200});
    }
    
    // Числовое значение справа
    std::stringstream ss;
    ss << (int)value << "/" << (int)maxValue;
    std::string valStr = ss.str();
    Vector2 vs = MeasureTextEx(font, valStr.c_str(), 11, 1);
    DrawTextEx(font, valStr.c_str(), {x + w - vs.x - 5, y + (h - vs.y) / 2}, 11, 1, Color{255, 255, 255, 220});
}

// Рисование иконки с текстом
inline void DrawStatIcon(float x, float y, const char* icon, const char* value, Color iconColor, Font& font) {
    DrawTextEx(font, icon, {x, y}, 16, 1, iconColor);
    DrawTextEx(font, value, {x + 22, y + 1}, 14, 1, Color{220, 220, 220, 255});
}

// Рисование мини-карты
inline void DrawMinimap(float x, float y, float size, 
                        Vector2 playerPos, Vector2 lakePos, float lakeRadius,
                        Vector2 campfirePos, Vector2 tentPos,
                        void* treesPtr, void* rocksPtr, void* slimesPtr,
                        int dayCount, Font& font) {
    
    // Тень под мини-картой
    DrawRectangleRounded({x + 3, y + 3, size, size}, 0.05f, 8, Color{0, 0, 0, 100});
    
    // Фон мини-карты
    DrawRectangleRounded({x, y, size, size}, 0.05f, 8, Color{25, 40, 30, 230});
    DrawRectangleRoundedLines({x, y, size, size}, 0.05f, 8, 2, Color{60, 80, 65, 255});
    
    float scale = size / 3000.0f;
    
    // Биомы (цветные зоны)
    // Лес - зелёный
    DrawRectangle(x + 2, y + 2, size * 0.3f, size * 0.3f, Color{30, 80, 40, 150});
    // Пустыня - жёлтый
    DrawRectangle(x + size * 0.7f - 2, y + 2, size * 0.3f, size * 0.3f, Color{120, 100, 50, 150});
    // Снег - голубой
    DrawRectangle(x + size * 0.7f - 2, y + size * 0.7f - 2, size * 0.3f, size * 0.3f, Color{180, 200, 220, 150});
    // Болото - тёмный
    DrawRectangle(x + 2, y + size * 0.7f - 2, size * 0.3f, size * 0.3f, Color{40, 50, 30, 150});
    
    // Озеро
    DrawCircle(x + lakePos.x * scale, y + lakePos.y * scale, lakeRadius * scale, Color{40, 100, 180, 180});
    DrawCircleLines(x + lakePos.x * scale, y + lakePos.y * scale, lakeRadius * scale, Color{60, 130, 200, 200});
    
    // Дороги (пунктир)
    for (int i = 0; i < 10; i++) {
        float t = i / 10.0f;
        float px = campfirePos.x + (tentPos.x - campfirePos.x) * t;
        float py = campfirePos.y + (tentPos.y - campfirePos.y) * t;
        DrawCircle(x + px * scale, y + py * scale, 1.5f, Color{100, 100, 100, 150});
    }
    
    // Лагерь (костёр)
    DrawCircle(x + campfirePos.x * scale, y + campfirePos.y * scale, 4, Color{245, 158, 11, 255});
    DrawCircle(x + campfirePos.x * scale, y + campfirePos.y * scale, 2, Color{255, 220, 100, 255});
    
    // Палатка
    DrawCircle(x + tentPos.x * scale, y + tentPos.y * scale, 3, Color{180, 83, 9, 255});
    
    // Деревья (маленькие точки)
    // Пропускаем для производительности
    
    // Игрок (мигающая точка)
    float pulse = sinf(GetTime() * 5) * 0.3f + 0.7f;
    DrawCircle(x + playerPos.x * scale, y + playerPos.y * scale, 5, Color{255, 255, 255, (unsigned char)(255 * pulse)});
    DrawCircle(x + playerPos.x * scale, y + playerPos.y * scale, 3, Color{100, 200, 255, 255});
    
    // Рамка биомов
    DrawLine(x + size * 0.5f, y, x + size * 0.5f, y + size, Color{60, 80, 65, 80});
    DrawLine(x, y + size * 0.5f, x + size, y + size * 0.5f, Color{60, 80, 65, 80});
    
    // Названия биомов
    DrawTextEx(font, "Лес", {x + 5, y + 5}, 9, 1, Color{100, 200, 120, 180});
    DrawTextEx(font, "Пустыня", {x + size - 45, y + 5}, 9, 1, Color{200, 180, 100, 180});
    DrawTextEx(font, "Снег", {x + size - 30, y + size - 15}, 9, 1, Color{200, 220, 240, 180});
    DrawTextEx(font, "Болото", {x + 5, y + size - 15}, 9, 1, Color{80, 120, 60, 180});
    
    // День
    std::stringstream daySS;
    daySS << "День " << dayCount;
    DrawTextEx(font, daySS.str().c_str(), {x + size / 2 - 15, y + size + 5}, 11, 1, Color{200, 200, 200, 200});
}

// Главная функция отрисовки HUD
inline void DrawGameHUD(Font& font, int health, int maxHealth, int hunger, int maxHunger,
                        int xp, int xpToNext, int level, int gold, int damage,
                        float timeOfDay, int dayCount,
                        Vector2 playerPos, Vector2 lakePos, float lakeRadius,
                        Vector2 campfirePos, Vector2 tentPos,
                        void* treesPtr, void* rocksPtr, void* slimesPtr) {
    
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    
    // ===== ВЕРХНЯЯ ПАНЕЛЬ =====
    // Фон панели с градиентом
    DrawRectangle(0, 0, sw, 90, Color{20, 22, 30, 240});
    DrawLine(0, 90, sw, 90, Color{60, 70, 90, 255});
    
    // ===== ИМЯ И КЛАСС =====
    // TODO: передать имя и класс
    
    // ===== БАРЫ (слева) =====
    float barX = 20;
    float barY = 12;
    float barW = 180;
    float barH = 16;
    float barGap = 22;
    
    // HP бар
    DrawFancyBar(barX, barY, barW, barH, health, maxHealth, 
                 Color{220, 50, 50, 255}, Color{60, 20, 20, 255}, "HP", font);
    
    // Голод бар
    DrawFancyBar(barX, barY + barGap, barW, barH, hunger, maxHunger,
                 Color{210, 170, 60, 255}, Color{60, 50, 20, 255}, "Голод", font);
    
    // XP бар
    DrawFancyBar(barX, barY + barGap * 2, barW, barH, xp, xpToNext,
                 Color{100, 180, 255, 255}, Color{25, 40, 60, 255}, "XP", font);
    
    // ===== СТАТЫ (по центру) =====
    float statX = 220;
    float statY = 15;
    
    // Уровень
    std::stringstream lvlSS;
    lvlSS << "Ур." << level;
    DrawTextEx(font, lvlSS.str().c_str(), {statX, statY}, 18, 1, Color{255, 220, 100, 255});
    
    // Золото
    std::stringstream goldSS;
    goldSS << gold;
    DrawStatIcon(statX + 70, statY, "$", goldSS.str().c_str(), Color{255, 200, 50, 255}, font);
    
    // Урон
    std::stringstream dmgSS;
    dmgSS << damage;
    DrawStatIcon(statX + 150, statY, "!", dmgSS.str().c_str(), Color{255, 100, 100, 255}, font);
    
    // ===== ВРЕМЯ (справа от центра) =====
    int h = (int)timeOfDay;
    int m = (int)((timeOfDay - h) * 60);
    std::stringstream timeSS;
    timeSS << (h < 10 ? "0" : "") << h << ":" << (m < 10 ? "0" : "") << m;
    
    const char* phaseNames[] = {"Утро", "День", "Вечер", "Ночь"};
    Color phaseColors[] = {
        Color{255, 200, 100, 255},  // Утро
        Color{255, 255, 200, 255},  // День
        Color{255, 150, 50, 255},   // Вечер
        Color{100, 150, 255, 255}   // Ночь
    };
    
    DrawTextEx(font, timeSS.str().c_str(), {sw - 200, 15}, 20, 1, Color{220, 220, 220, 255});
    DrawTextEx(font, phaseNames[0], {sw - 130, 18}, 14, 1, phaseColors[0]); // TODO: правильная фаза
    
    // ===== МИНИ-КАРТА (справа) =====
    float mmSize = 140;
    float mmX = sw - mmSize - 15;
    float mmY = 45;
    DrawMinimap(mmX, mmY, mmSize, playerPos, lakePos, lakeRadius, campfirePos, tentPos,
                treesPtr, rocksPtr, slimesPtr, dayCount, font);
    
    // ===== ПОДСКАЗКИ (снизу) =====
    DrawRectangle(0, sh - 30, sw, 30, Color{20, 22, 30, 200});
    DrawTextEx(font, "[WASD] бег  [E] добыча  [F] рыбалка  [T] торговля  [I] инвентарь  [C] крафт  [F11] полноэкранный", 
               {15, sh - 22}, 12, 1, Color{150, 150, 160, 200});
}
