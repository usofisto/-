#include "render.h"
#include "resource_manager.h"
#include "particles.h"
#include <cmath>

void DrawTextureCentered(Texture2D tex, Vector2 position, float scale, float rotation) {
    if (tex.id == 0) return;
    Rectangle source = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    Rectangle dest = { position.x, position.y, tex.width * scale, tex.height * scale };
    Vector2 origin = { (tex.width * scale) / 2.0f, (tex.height * scale) / 2.0f };
    DrawTexturePro(tex, source, dest, origin, rotation, WHITE);
}

// Рисуем персонажа: направление взгляда, шагающие ноги, экипировка
void DrawPlayerAvatar(const std::string& className, Vector2 position, float radius, Vector2 facing, bool isMoving, int framesCounter, Color bgDark) {
    Texture2D tex = ResourceManager::Get().GetTex("player");
    if (tex.id != 0) {
        // === АНИМАЦИЯ ПЕРСОНАЖА ===
        float bobbing = 0.0f;
        float rotation = 0.0f;
        float scale = 1.0f;
        
        if (isMoving) {
            // При ходьбе: покачивание + лёгкое покачивание корпуса
            bobbing = sinf(framesCounter * 0.3f) * 3.0f;
            rotation = sinf(framesCounter * 0.2f) * 3.0f; // Покачивание корпуса
        } else {
            // В покое: эффект "дыхания" - лёгкая пульсация
            bobbing = sinf(framesCounter * 0.08f) * 1.0f;
            scale = 1.0f + sinf(framesCounter * 0.1f) * 0.02f; // Микро-пульсация
        }
        
        Vector2 renderPos = { position.x, position.y + bobbing };
        
        // Тень под спрайтом (пульсирует при движении)
        float shadowSize = isMoving ? 12.0f + sinf(framesCounter * 0.3f) * 2.0f : 12.0f;
        DrawEllipse(position.x, position.y + 16, shadowSize, 5, Color{ 0, 0, 0, 120 });
        
        // Флип спрайта в зависимости от направления
        Rectangle source = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        if (facing.x < 0) source.width = -source.width;
        
        // Масштабируем с учётом анимации
        float size = 48.0f * scale;
        Rectangle dest = { renderPos.x, renderPos.y, size, size };
        Vector2 origin = { size / 2.0f, size / 2.0f };
        DrawTexturePro(tex, source, dest, origin, rotation, WHITE);
        return;
    }
    
    // Фаза анимации движения ног и покачивания туловища (Fallback для геометрической отрисовки)
    float bobbingY = 0.0f;
    float footOffset = 0.0f;
    if (isMoving) {
        bobbingY = sinf(framesCounter * 0.22f) * 2.5f;
        footOffset = sinf(framesCounter * 0.22f) * 5.5f;
    }
    
    Vector2 center = { position.x, position.y + bobbingY };
    
    // Тень под персонажем
    DrawEllipse(center.x, center.y + 14, 11, 4.5f, Color{ 0, 0, 0, 120 });
    
    // Выбор цветов экипировки по классу персонажа
    Color tunicColor = Color{ 100, 100, 100, 255 };
    Color tunicDarkColor = Color{ 70, 70, 70, 255 };
    Color hatColor = Color{ 50, 50, 50, 255 };
    
    if (className == "Воин") {
        tunicColor = Color{ 59, 130, 246, 255 }; // Синяя броня
        tunicDarkColor = Color{ 29, 78, 216, 255 };
        hatColor = Color{ 156, 163, 175, 255 }; // Стальной шлем
    } else if (className == "Маг") {
        tunicColor = Color{ 139, 92, 246, 255 }; // Фиолетовая мантия
        tunicDarkColor = Color{ 109, 40, 217, 255 };
        hatColor = Color{ 79, 70, 229, 255 }; // Индиго шляпа
    } else if (className == "Лучник") {
        tunicColor = Color{ 16, 185, 129, 255 }; // Зеленый кожаный доспех
        tunicDarkColor = Color{ 4, 120, 87, 255 };
        hatColor = Color{ 180, 83, 9, 255 }; // Коричневый капюшон
    }
    
    // 1. Отрисовка ног (шагание)
    Vector2 leftFoot = { center.x - 5, center.y + 12 };
    Vector2 rightFoot = { center.x + 5, center.y + 12 };
    
    if (isMoving) {
        if (fabsf(facing.x) > fabsf(facing.y)) {
            // Движение по горизонтали: ноги двигаются вверх/вниз
            leftFoot.y += footOffset;
            rightFoot.y -= footOffset;
        } else {
            // Движение по вертикали: ноги двигаются влево/вправо
            leftFoot.x += footOffset * 0.7f;
            rightFoot.x -= footOffset * 0.7f;
        }
    }
    
    DrawCircleV(leftFoot, 3.5f, Color{ 78, 53, 36, 255 }); // Кожаные ботинки
    DrawCircleV(rightFoot, 3.5f, Color{ 78, 53, 36, 255 });
    
    // 2. Отрисовка тела (туника / нагрудник)
    Rectangle bodyRect = { center.x - 9, center.y - 5, 18, 15 };
    DrawRectangleRounded(bodyRect, 0.35f, 4, tunicColor);
    DrawRectangleRoundedLines(bodyRect, 0.35f, 4, 1.5f, tunicDarkColor);
    
    if (className == "Воин") {
        // Золотой герб на груди рыцаря
        DrawCircle(center.x, center.y + 2, 2.5f, Color{ 245, 158, 11, 255 });
    } else if (className == "Лучник") {
        // Кожаный ремень через плечо
        DrawLineEx(Vector2{ center.x - 9, center.y - 3 }, Vector2{ center.x + 9, center.y + 7 }, 2.0f, Color{ 120, 53, 4, 255 });
    }
    
    // 3. Отрисовка головы и лица (зависит от направления)
    Vector2 headCenter = { center.x, center.y - 10 };
    DrawCircleV(headCenter, 8.5f, Color{ 254, 215, 170, 255 }); // Цвет кожи
    
    // Глаза направляются в сторону взгляда
    if (facing.y > 0.0f) {
        // Смотрит вниз: оба глаза видны прямо
        DrawCircle(headCenter.x - 3, headCenter.y - 1, 2, Color{ 255, 255, 255, 255 });
        DrawCircle(headCenter.x + 3, headCenter.y - 1, 2, Color{ 255, 255, 255, 255 });
        DrawCircle(headCenter.x - 3, headCenter.y - 1, 1, Color{ 0, 0, 0, 255 });
        DrawCircle(headCenter.x + 3, headCenter.y - 1, 1, Color{ 0, 0, 0, 255 });
    } 
    else if (facing.x < 0.0f) {
        // Смотрит влево: один глаз виден сбоку
        DrawCircle(headCenter.x - 5, headCenter.y - 1, 2, Color{ 255, 255, 255, 255 });
        DrawCircle(headCenter.x - 5, headCenter.y - 1, 1, Color{ 0, 0, 0, 255 });
    } 
    else if (facing.x > 0.0f) {
        // Смотрит вправо: один глаз виден сбоку
        DrawCircle(headCenter.x + 5, headCenter.y - 1, 2, Color{ 255, 255, 255, 255 });
        DrawCircle(headCenter.x + 5, headCenter.y - 1, 1, Color{ 0, 0, 0, 255 });
    }
    // Смотрит вверх: лица не видно (голова затылком)
    
    // 4. Отрисовка головного убора
    if (className == "Воин") {
        // Рыцарский шлем с красным пером
        DrawCircleSector(headCenter, 8.5f, 180, 360, 6, hatColor);
        DrawRectangle(headCenter.x - 8.5f, headCenter.y - 1, 17, 3, hatColor);
        
        float plumeSway = isMoving ? sinf(framesCounter * 0.25f) * 2.5f : 0.0f;
        DrawTriangle(
            Vector2{ headCenter.x, headCenter.y - 8 },
            Vector2{ headCenter.x - 10 + plumeSway, headCenter.y - 17 },
            Vector2{ headCenter.x - 2 + plumeSway, headCenter.y - 8 },
            Color{ 220, 38, 38, 255 }
        );
    } 
    else if (className == "Маг") {
        // Высокая остроконечная шляпа с пряжкой
        DrawTriangle(
            Vector2{ headCenter.x, headCenter.y - 25 },
            Vector2{ headCenter.x - 12, headCenter.y - 6 },
            Vector2{ headCenter.x + 12, headCenter.y - 6 },
            hatColor
        );
        DrawEllipse(headCenter.x, headCenter.y - 6, 15, 2.5f, hatColor);
        DrawRectangle(headCenter.x - 3, headCenter.y - 9, 6, 3, Color{ 245, 158, 11, 255 });
    } 
    else if (className == "Лучник") {
        // Капюшон следопыта
        DrawCircleSector(headCenter, 9.5f, 160, 380, 6, tunicColor);
        DrawEllipse(headCenter.x, headCenter.y + 5, 10, 2.5f, tunicColor);
    }
    
    // 5. Оружие и щит в руках
    Vector2 handPos = { center.x + 11, center.y + 2 };
    if (facing.x < 0.0f) {
        handPos = { center.x - 11, center.y + 2 };
    }
    if (facing.y < 0.0f) {
        handPos.y -= 4; // Подъем рук при ходьбе вверх
    }
    
    if (className == "Воин") {
        // Рисуем щит на противоположной руке
        Vector2 shieldPos = { center.x - 11, center.y + 3 };
        if (facing.x < 0.0f) shieldPos = { center.x + 11, center.y + 3 };
        DrawCircleV(shieldPos, 6.5f, Color{ 185, 28, 28, 255 });
        DrawCircleV(shieldPos, 4.0f, Color{ 220, 38, 38, 255 });
        
        // Меч
        Vector2 swordTip = { handPos.x + 8, handPos.y - 13 };
        if (facing.x < 0.0f) swordTip = { handPos.x - 8, handPos.y - 13 };
        if (facing.y < 0.0f) swordTip = { handPos.x, handPos.y - 15 };
        
        DrawLineEx(handPos, swordTip, 3.0f, Color{ 229, 231, 235, 255 });
        DrawLineEx(Vector2{ handPos.x - 2, handPos.y - 3 }, Vector2{ handPos.x + 2, handPos.y - 1 }, 2, Color{ 120, 53, 4, 255 });
    } 
    else if (className == "Маг") {
        // Магический светящийся посох
        Vector2 staffTop = { handPos.x + 3, handPos.y - 16 };
        if (facing.x < 0.0f) staffTop = { handPos.x - 3, handPos.y - 16 };
        
        DrawLineEx(Vector2{ handPos.x, handPos.y + 7 }, staffTop, 2.0f, Color{ 120, 53, 4, 255 });
        DrawCircleV(staffTop, 3.5f, Color{ 253, 224, 71, 255 });
        DrawCircleV(staffTop, 6.0f, Color{ 253, 224, 71, 80 });
    } 
    else if (className == "Лучник") {
        // Деревянный лук
        DrawCircleSector(handPos, 10, -90, 90, 5, Color{ 180, 83, 9, 255 });
        DrawCircleSector(handPos, 8, -90, 90, 5, bgDark);
        DrawLine(handPos.x, handPos.y - 10, handPos.x, handPos.y + 10, Color{ 243, 244, 246, 120 });
    }
}

// Дерево: ствол с корой, объёмная крона с тенью и бликами
void DrawTree(Vector2 position, float radius) {
    // Ствол: внешняя кора + ядро
    DrawRectangle(position.x - 7, position.y - 2, 14, 36, Color{ 55, 33, 18, 255 });
    DrawRectangle(position.x - 5, position.y - 2, 10, 36, Color{ 68, 43, 26, 255 });
    DrawRectangle(position.x - 3, position.y, 6, 32, Color{ 120, 53, 4, 255 });
    
    // Волокна коры для фактуры
    DrawLine(position.x - 4, position.y + 3, position.x - 4, position.y + 28, Color{ 45, 25, 10, 255 });
    DrawLine(position.x - 1, position.y + 6, position.x - 1, position.y + 26, Color{ 80, 35, 10, 255 });
    DrawLine(position.x + 2, position.y + 8, position.x + 2, position.y + 22, Color{ 80, 35, 10, 255 });
    DrawLine(position.x + 4, position.y + 4, position.x + 4, position.y + 27, Color{ 45, 25, 10, 255 });

    // Корни у основания ствола
    DrawTriangle(
        Vector2{ position.x - 7, position.y + 34 },
        Vector2{ position.x - 14, position.y + 38 },
        Vector2{ position.x - 3, position.y + 36 },
        Color{ 55, 33, 18, 255 }
    );
    DrawTriangle(
        Vector2{ position.x + 7, position.y + 34 },
        Vector2{ position.x + 3, position.y + 36 },
        Vector2{ position.x + 14, position.y + 38 },
        Color{ 55, 33, 18, 255 }
    );

    // Крона: более естественная форма из нескольких кругов
    // Тень кроны (самый нижний слой)
    DrawCircle(position.x + 3, position.y - 8, radius + 6, Color{ 14, 60, 32, 255 });
    DrawCircle(position.x - 5, position.y - 12, radius + 5, Color{ 14, 60, 32, 255 });
    
    // Тёмный контур кроны
    DrawCircle(position.x, position.y - 13, radius + 4, Color{ 20, 83, 45, 255 });
    DrawCircle(position.x - 4, position.y - 15, radius + 3, Color{ 22, 101, 52, 255 });
    
    // Основная масса листвы - несколько перекрывающихся кругов
    DrawCircle(position.x, position.y - 14, radius, Color{ 22, 163, 74, 255 });
    DrawCircle(position.x - 6, position.y - 16, radius - 2, Color{ 34, 197, 94, 255 });
    DrawCircle(position.x + 5, position.y - 12, radius - 3, Color{ 21, 128, 61, 255 });
    
    // Боковые кластеры для объёма
    DrawCircle(position.x - 8, position.y - 16, radius - 3, Color{ 34, 197, 94, 255 });
    DrawCircle(position.x + 7, position.y - 10, radius - 4, Color{ 21, 128, 61, 255 });
    DrawCircle(position.x - 3, position.y - 20, radius - 5, Color{ 74, 222, 128, 255 });
    
    // Контрастные блики (солнечный свет)
    DrawCircle(position.x - 10, position.y - 22, 3.0f, Color{ 187, 247, 208, 230 });
    DrawCircle(position.x - 5, position.y - 18, 2.0f, Color{ 200, 255, 220, 180 });
    DrawCircle(position.x + 8, position.y - 14, 2.5f, Color{ 187, 247, 208, 180 });
    
    // Чёткий внешний контур кроны (обводка)
    DrawCircleLines(position.x, position.y - 13, radius + 4, Color{ 14, 60, 32, 200 });
}

// Декоративный каменный бортик костра
void DrawFirePit(Vector2 position) {
    int numStones = 8;
    for (int i = 0; i < numStones; i++) {
        float angle = i * (3.14159f * 2.0f / numStones);
        Vector2 stonePos = { position.x + cosf(angle) * 19.0f, position.y + sinf(angle) * 19.0f };
        DrawCircleV(stonePos, 4.5f, Color{ 113, 113, 122, 255 }); // Серый камень
        DrawCircleV(stonePos, 3.5f, Color{ 161, 161, 170, 255 }); // Серый блик
        DrawCircleLines(stonePos.x, stonePos.y, 4.5f, Color{ 63, 63, 70, 255 }); // Контур
    }
}

// Рисование мерцающего костра в безопасном лагере
void DrawCampfire(Vector2 position, int framesCounter) {
    DrawFirePit(position);
    
    DrawLineEx(Vector2{ position.x - 10, position.y + 6 }, Vector2{ position.x + 10, position.y - 3 }, 4, Color{ 85, 40, 15, 255 });
    DrawLineEx(Vector2{ position.x + 10, position.y + 6 }, Vector2{ position.x - 10, position.y - 3 }, 4, Color{ 85, 40, 15, 255 });
    
    float f1 = 10.0f + 2.0f * sinf(framesCounter * 0.25f);
    float f2 = 6.5f + 1.5f * sinf(framesCounter * 0.35f + 1.0f);
    float f3 = 3.5f + 0.8f * sinf(framesCounter * 0.45f + 2.0f);
    
    DrawCircleV(position, f1, Color{ 239, 68, 68, 180 });  // Внешнее пламя
    DrawCircleV(position, f2, Color{ 245, 158, 11, 210 }); // Среднее пламя
    DrawCircleV(position, f3, Color{ 253, 224, 71, 255 }); // Ядро
}

// Рисование палатки для отдыха в лагере
void DrawTent(Vector2 position) {
    DrawTriangle(
        Vector2{ position.x, position.y - 35 },
        Vector2{ position.x - 40, position.y + 12 },
        Vector2{ position.x + 40, position.y + 12 },
        Color{ 68, 43, 26, 255 }
    );
    DrawTriangle(
        Vector2{ position.x - 4, position.y - 35 },
        Vector2{ position.x - 40, position.y + 12 },
        Vector2{ position.x + 12, position.y + 12 },
        Color{ 180, 83, 9, 255 }
    );
    DrawTriangle(
        Vector2{ position.x, position.y - 12 },
        Vector2{ position.x - 12, position.y + 12 },
        Vector2{ position.x + 12, position.y + 12 },
        Color{ 24, 24, 27, 255 }
    );
}

// Отрисовка озера (воды) на карте - естественная форма
void DrawWaterPond(Vector2 position, float radius, int framesCounter) {
    // Основная форма озера - несколько перекрывающихся эллипсов для естественности
    DrawEllipse((int)position.x, (int)position.y, radius, radius * 0.85f, Color{ 30, 58, 138, 255 });
    DrawEllipse((int)position.x - 15, (int)position.y + 10, radius * 0.7f, radius * 0.6f, Color{ 29, 78, 216, 255 });
    DrawEllipse((int)position.x + 20, (int)position.y - 5, radius * 0.6f, radius * 0.5f, Color{ 37, 99, 235, 255 });
    
    // Волны на поверхности
    float waveTime = framesCounter * 0.03f;
    for (int i = 0; i < 3; i++) {
        float waveOffset = sinf(waveTime + i * 2.0f) * 8.0f;
        float waveY = position.y - 20 + i * 15 + waveOffset;
        DrawEllipse((int)position.x, (int)waveY, radius * 0.4f, 3, Color{ 147, 197, 253, 40 + i * 15 });
    }
    
    // Блики на воде
    float shimmer = sinf(framesCounter * 0.05f) * 0.3f + 0.7f;
    DrawEllipse((int)position.x - 25, (int)position.y - 30, 20, 6, ColorAlpha(Color{ 191, 219, 254, 255 }, shimmer * 0.6f));
    DrawEllipse((int)position.x + 15, (int)position.y - 20, 12, 4, ColorAlpha(Color{ 191, 219, 254, 255 }, shimmer * 0.4f));
    
    // Камыш у берега
    DrawLineEx(Vector2{ position.x - radius + 10, position.y + 20 }, Vector2{ position.x - radius + 8, position.y - 15 }, 2.0f, Color{ 34, 197, 94, 180 });
    DrawLineEx(Vector2{ position.x + radius - 15, position.y + 25 }, Vector2{ position.x + radius - 12, position.y - 10 }, 2.0f, Color{ 34, 197, 94, 180 });
    DrawCircleV(Vector2{ position.x - radius + 8, position.y - 15 }, 3, Color{ 132, 204, 22, 200 });
    DrawCircleV(Vector2{ position.x + radius - 12, position.y - 10 }, 3, Color{ 132, 204, 22, 200 });
}

// Отрисовка каменных стен руин
void DrawStoneWall(const StoneWall& wall) {
    DrawRectangleRec(wall.rect, Color{ 63, 63, 70, 255 });
    Rectangle inner = { wall.rect.x + 2, wall.rect.y + 2, wall.rect.width - 4, wall.rect.height - 4 };
    DrawRectangleRec(inner, Color{ 113, 113, 122, 255 });
    
    if (wall.rect.width > wall.rect.height) {
        for (float x = wall.rect.x + 30; x < (wall.rect.x + wall.rect.width); x += 30) {
            DrawLine(x, wall.rect.y + 2, x, wall.rect.y + wall.rect.height - 2, Color{ 39, 39, 42, 255 });
        }
    } else {
        for (float y = wall.rect.y + 30; y < (wall.rect.y + wall.rect.height); y += 30) {
            DrawLine(wall.rect.x + 2, y, wall.rect.x + wall.rect.width - 2, y, Color{ 39, 39, 42, 255 });
        }
    }
}

// Рисование собираемых предметов (трава, золотоцвет)
void DrawMapItem(const MapItem& item) {
    if (!item.active) return;
    
    if (item.type == ITEM_HERB) {
        DrawCircleV(item.position, 7, Color{ 34, 197, 94, 140 });
        DrawCircleV(item.position, 4, Color{ 34, 197, 94, 255 });
        DrawRectangle(item.position.x - 1.5f, item.position.y - 5, 3, 10, Color{ 255, 255, 255, 200 });
        DrawRectangle(item.position.x - 5, item.position.y - 1.5f, 10, 3, Color{ 255, 255, 255, 200 });
    } 
    else if (item.type == ITEM_GOLDFLOWER) {
        DrawCircleV(item.position, 7, Color{ 245, 158, 11, 110 });
        DrawCircleV(item.position, 4, Color{ 253, 224, 71, 255 });
        DrawCircle(item.position.x, item.position.y, 1.5f, Color{ 255, 255, 255, 255 });
    }
}

// Слизень: глянцевое тело, блики, глаза следят за направлением
void DrawWanderingSlime(const WanderingSlime& slime, int framesCounter, Color bgDark) {
    if (!slime.active) return;
    
    // Выбираем текстуру по типу слизня
    std::string texKey = "slime";
    switch (slime.slimeType) {
        case 1: texKey = "slime_green"; break;
        case 2: texKey = "slime_red"; break;
        case 3: texKey = "slime_blue"; break;
    }
    Texture2D tex = ResourceManager::Get().GetTex(texKey.c_str());
    if (tex.id == 0) tex = ResourceManager::Get().GetTex("slime"); // фоллбэк
    
    if (tex.id != 0) {
        // === АНИМАЦИЯ СЛИЗНЯ ===
        float bounce = sinf(framesCounter * 0.16f + slime.id) * 3.2f;
        float squish = sinf(framesCounter * 0.16f + slime.id + 0.5f) * 0.15f;
        Vector2 renderPos = { slime.position.x, slime.position.y + bounce * 0.4f };
        
        // Тень с деформацией
        float shadowSquish = 1.0f + squish * 0.5f;
        DrawEllipse(slime.position.x, slime.position.y + 16, 14 * shadowSquish, 5, Color{ 0, 0, 0, 110 });
        
        Rectangle source = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        if (slime.targetPosition.x < slime.position.x) source.width = -source.width;
        
        float scaleX = 48.0f * (1.0f + squish);
        float scaleY = 48.0f * (1.0f - squish);
        Rectangle dest = { renderPos.x, renderPos.y, scaleX, scaleY };
        Vector2 origin = { scaleX / 2.0f, scaleY / 2.0f };
        
        DrawTexturePro(tex, source, dest, origin, 0.0f, WHITE);
        return;
    }
    
    // Fallback процедурный
    float bounce = sinf(framesCounter * 0.16f + slime.id) * 3.2f;
    float rx = 15.5f + bounce * 0.5f;
    float ry = 12.5f - bounce * 0.5f;
    
    Vector2 slimePos = { slime.position.x, slime.position.y + bounce * 0.4f };
    
    DrawEllipse(slime.position.x, slime.position.y + 10, rx * 0.85f, 4, Color{ 0, 0, 0, 110 });
    
    Color sColor, sGlow, sOutline;
    switch (slime.slimeType) {
        case 2: // красный
            sColor = Color{ 220, 38, 38, 255 };
            sGlow = Color{ 248, 113, 113, 160 };
            sOutline = Color{ 127, 29, 29, 255 };
            break;
        case 3: // синий
            sColor = Color{ 59, 130, 246, 255 };
            sGlow = Color{ 96, 165, 250, 160 };
            sOutline = Color{ 29, 78, 137, 255 };
            break;
        default: // зелёный
            sColor = Color{ 34, 197, 94, 255 };
            sGlow = Color{ 74, 222, 128, 160 };
            sOutline = Color{ 20, 83, 45, 255 };
            break;
    }
    
    DrawEllipse((int)slimePos.x, (int)slimePos.y, rx + 2, ry + 2, sOutline);
    DrawEllipse((int)slimePos.x, (int)slimePos.y, rx, ry, sColor);
    DrawEllipse((int)slimePos.x, (int)slimePos.y, rx - 3, ry - 2, ColorAlpha(sGlow, 0.45f));
    
    // Блики
    DrawEllipse((int)slimePos.x - rx * 0.3f, (int)slimePos.y - ry * 0.3f, rx * 0.22f, ry * 0.18f, Color{ 255, 255, 255, 200 });
    
    // Глаза
    DrawCircle((int)slimePos.x - 8, (int)slimePos.y - 3, 5, WHITE);
    DrawCircle((int)slimePos.x + 8, (int)slimePos.y - 3, 5, WHITE);
    DrawCircle((int)slimePos.x - 8, (int)slimePos.y - 3, 2, bgDark);
    DrawCircle((int)slimePos.x + 8, (int)slimePos.y - 3, 2, bgDark);
    
    // Частицы
    if (rand() % 100 < 8) {
        SpawnParticle(slime.position, { 0, 0 }, ColorAlpha(sGlow, 160), 3 + rand() % 3, 0.6f + (rand() % 5) / 10.f);
    }
}
