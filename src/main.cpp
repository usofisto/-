#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "raylib.h"
#include "player.h"
#include "biom.h"

// ==========================================
// 1. ИГРОВЫЕ СОСТОЯНИЯ И КОНСТАНТЫ
// ==========================================
enum GameState {
    STATE_WELCOME,         // Ввод имени персонажа
    STATE_CLASS_SELECT,    // Выбор класса
    STATE_2D_WORLD,        // Свободное исследование 2D мира (Лагерь и Поляна объединены)
    STATE_MEADOW_SLIME,    // Полноценный графический пошаговый бой со слизью
    STATE_GAME_OVER        // Герой погиб в приключении
};

// Типы собираемых предметов на карте
enum ItemType {
    ITEM_HERB,         // Лечебная трава (+HP)
    ITEM_GOLDFLOWER    // Золотоцвет (+Золото)
};

// ==========================================
// 2. СТРУКТУРЫ ДЛЯ ДВУМЕРНОГО МИРА
// ==========================================

// Собираемый предмет на карте
struct MapItem {
    Vector2 position;
    ItemType type;
    bool active;
    float respawnTimer;
};

// Препятствие (Дерево, камни) для коллизий
struct Obstacle {
    Vector2 position;
    float radius;
};

// Блуждающий слизень (враг на карте)
struct WanderingSlime {
    Vector2 position;
    Vector2 targetPosition;
    float wanderTimer;
    float speed;
    int hp;
    int maxHp;
    bool active;
    int id; // Уникальный идентификатор слизня
};

// Всплывающий летающий текст (урон, золото, лечение)
struct FloatingText {
    std::string text;
    Vector2 position;
    Color color;
    float alpha;
    float ySpeed;
    float lifetime;
    bool active;
};

// ==========================================
// 3. ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ И ОТРИСОВКА GUI
// ==========================================

// Функция для кодирования юникод-кодов в UTF-8 (для ввода кириллицы)
void AppendUnicodeToUTF8(std::string& str, int codepoint) {
    if (codepoint <= 0x7F) {
        str += (char)codepoint;
    } else if (codepoint <= 0x7FF) {
        str += (char)(0xC0 | ((codepoint >> 6) & 0x1F));
        str += (char)(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        str += (char)(0xE0 | ((codepoint >> 12) & 0x0F));
        str += (char)(0x80 | ((codepoint >> 6) & 0x3F));
        str += (char)(0x80 | (codepoint & 0x3F));
    }
}

// Добавление сообщения в мини-лог событий
void AddLogMessage(const std::string& msg, std::vector<std::pair<std::string, float>>& log) {
    log.push_back({ msg, 4.0f }); // Сообщение активно 4 секунды
    if (log.size() > 5) {
        log.erase(log.begin());
    }
}

// Отрисовка текста с автопереносом строк (UTF-8)
void DrawWrappedText(Font font, const char* text, int posX, int posY, int maxWidth, int fontSize, Color color) {
    std::string textStr(text);
    std::vector<std::string> words;
    std::string currentWord = "";
    
    for (size_t i = 0; i < textStr.length(); ++i) {
        if (textStr[i] == ' ') {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord = "";
            }
        } else if (textStr[i] == '\n') {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord = "";
            }
            words.push_back("\n");
        } else {
            currentWord += textStr[i];
        }
    }
    if (!currentWord.empty()) {
        words.push_back(currentWord);
    }
    
    std::string currentLine = "";
    int currentY = posY;
    
    for (const auto& word : words) {
        if (word == "\n") {
            DrawTextEx(font, currentLine.c_str(), Vector2{ (float)posX, (float)currentY }, (float)fontSize, 1.0f, color);
            currentY += fontSize + 4;
            currentLine = "";
            continue;
        }
        
        std::string testLine = currentLine + (currentLine.empty() ? "" : " ") + word;
        Vector2 size = MeasureTextEx(font, testLine.c_str(), (float)fontSize, 1.0f);
        if (size.x > maxWidth) {
            DrawTextEx(font, currentLine.c_str(), Vector2{ (float)posX, (float)currentY }, (float)fontSize, 1.0f, color);
            currentY += fontSize + 4;
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty()) {
        DrawTextEx(font, currentLine.c_str(), Vector2{ (float)posX, (float)currentY }, (float)fontSize, 1.0f, color);
    }
}

// Отрисовка кнопки с визуальным откликом при наведении и клике
bool DrawButton(Font font, Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color clickColor, Color textColor) {
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rect);
    bool clicked = false;
    Color drawColor = baseColor;
    
    if (hovered) {
        drawColor = hoverColor;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            drawColor = clickColor;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            clicked = true;
        }
    }
    
    DrawRectangleRounded(rect, 0.25f, 4, drawColor);
    DrawRectangleRoundedLines(rect, 0.25f, 4, 1.5f, Color{ 255, 255, 255, 30 });
    
    float fontSize = 18.0f;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1.0f);
    DrawTextEx(font, text, Vector2{ rect.x + (rect.width - textSize.x)/2.0f, rect.y + (rect.height - textSize.y)/2.0f }, fontSize, 1.0f, textColor);
    
    return clicked;
}

// Отрисовка прогресс-бара HP
void DrawProgressBar(Rectangle rect, float value, float maxValue, Color barColor, Color bgColor) {
    DrawRectangleRounded(rect, 0.3f, 4, bgColor);
    float fillWidth = (value / maxValue) * rect.width;
    if (fillWidth < 0.0f) fillWidth = 0.0f;
    if (fillWidth > rect.width) fillWidth = rect.width;
    
    if (fillWidth > 0.0f) {
        Rectangle fillRect = { rect.x, rect.y, fillWidth, rect.height };
        DrawRectangleRounded(fillRect, 0.3f, 4, barColor);
    }
    DrawRectangleRoundedLines(rect, 0.3f, 4, 1.5f, Color{ 255, 255, 255, 50 });
}

// Отрисовка карточки класса при создании персонажа
bool DrawClassCard(Font font, Rectangle rect, const char* title, const char* stats, const char* desc, bool selected, Color cardBgColor, Color borderColor) {
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rect);
    bool clicked = false;
    
    if (hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked = true;
    }
    
    Color bg = cardBgColor;
    Color border = borderColor;
    if (hovered) {
        bg = Color{ 45, 50, 62, 255 };
        border = Color{ 99, 102, 241, 200 };
    }
    if (selected) {
        bg = Color{ 50, 56, 70, 255 };
        border = Color{ 99, 102, 241, 255 };
    }
    
    DrawRectangleRounded(rect, 0.1f, 4, bg);
    DrawRectangleRoundedLines(rect, 0.1f, 4, 2.0f, border);
    
    DrawTextEx(font, title, Vector2{ rect.x + 20, rect.y + 20 }, 22.0f, 1.0f, Color{ 243, 244, 246, 255 });
    DrawTextEx(font, stats, Vector2{ rect.x + 20, rect.y + 55 }, 16.0f, 1.0f, Color{ 245, 158, 11, 255 });
    DrawWrappedText(font, desc, rect.x + 20, rect.y + 90, rect.width - 40, 14, Color{ 156, 163, 175, 255 });
    
    return clicked;
}

// ==========================================
// 4. ГРАФИЧЕСКАЯ ОТРИСОВКА ОБЪЕКТОВ МИРА
// ==========================================

// Рисование визуального аватара игрока в зависимости от выбранного класса
void DrawPlayerAvatar(std::string className, Vector2 position, float radius, float bobbingY, Color bgDark) {
    Vector2 center = { position.x, position.y + bobbingY };
    
    if (className == "Воин") {
        // Синий круг воина со стальным мечом и красным щитом
        DrawCircleV(center, radius, Color{ 59, 130, 246, 255 });
        DrawCircleV(center, radius - 4, Color{ 37, 99, 235, 255 });
        // Меч
        DrawLineEx(Vector2{ center.x + 10, center.y - 12 }, Vector2{ center.x + 24, center.y - 26 }, 4, Color{ 209, 213, 219, 255 });
        DrawLineEx(Vector2{ center.x + 8, center.y - 14 }, Vector2{ center.x + 15, center.y - 8 }, 3, Color{ 120, 53, 4, 255 });
        // Щит
        DrawCircleV(Vector2{ center.x - 14, center.y + 4 }, 9, Color{ 185, 28, 28, 255 });
        DrawCircleV(Vector2{ center.x - 14, center.y + 4 }, 6, Color{ 220, 38, 38, 255 });
    } 
    else if (className == "Маг") {
        // Фиолетовый маг в колдовской шляпе и со светящимся золотым посохом
        DrawCircleV(center, radius, Color{ 139, 92, 246, 255 });
        DrawCircleV(center, radius - 4, Color{ 124, 58, 237, 255 });
        // Шляпа
        DrawTriangle(
            Vector2{ center.x, center.y - 32 },
            Vector2{ center.x - 16, center.y - 10 },
            Vector2{ center.x + 16, center.y - 10 },
            Color{ 79, 70, 229, 255 }
        );
        // Посох
        DrawLineEx(Vector2{ center.x + 14, center.y + 12 }, Vector2{ center.x + 22, center.y - 20 }, 3, Color{ 120, 53, 4, 255 });
        DrawCircleV(Vector2{ center.x + 22, center.y - 20 }, 5, Color{ 253, 224, 71, 255 });
        DrawCircleV(Vector2{ center.x + 22, center.y - 20 }, 9, Color{ 253, 224, 71, 100 });
    } 
    else if (className == "Лучник") {
        // Зеленый лучник с деревянным луком за плечом
        DrawCircleV(center, radius, Color{ 16, 185, 129, 255 });
        DrawCircleV(center, radius - 4, Color{ 5, 150, 105, 255 });
        // Лук
        DrawCircleSector(Vector2{ center.x + 14, center.y }, 16, -90, 90, 6, Color{ 180, 83, 9, 255 });
        DrawCircleSector(Vector2{ center.x + 14, center.y }, 13, -90, 90, 6, bgDark);
        DrawLine(center.x + 14, center.y - 16, center.x + 14, center.y + 16, Color{ 243, 244, 246, 150 });
    }
    
    // Глаза персонажа
    DrawCircle(center.x - 5, center.y - 2, 3, Color{ 255, 255, 255, 255 });
    DrawCircle(center.x + 5, center.y - 2, 3, Color{ 255, 255, 255, 255 });
    DrawCircle(center.x - 5, center.y - 2, 1, Color{ 0, 0, 0, 255 });
    DrawCircle(center.x + 5, center.y - 2, 1, Color{ 0, 0, 0, 255 });
}

// Рисование дерева
void DrawTree(Vector2 position, float radius) {
    // Ствол дерева
    DrawRectangle(position.x - 5, position.y, 10, 30, Color{ 115, 60, 30, 255 });
    // Пышная листва (перекрывающиеся круги)
    DrawCircle(position.x, position.y - 12, radius + 4, Color{ 22, 101, 52, 220 });
    DrawCircle(position.x - 10, position.y - 4, radius - 2, Color{ 21, 128, 61, 200 });
    DrawCircle(position.x + 10, position.y - 4, radius - 2, Color{ 21, 128, 61, 200 });
}

// Рисование мерцающего костра в безопасном лагере
void DrawCampfire(Vector2 position, int framesCounter) {
    // Угли/дрова
    DrawLineEx(Vector2{ position.x - 12, position.y + 8 }, Vector2{ position.x + 12, position.y - 4 }, 4, Color{ 120, 53, 4, 255 });
    DrawLineEx(Vector2{ position.x + 12, position.y + 8 }, Vector2{ position.x - 12, position.y - 4 }, 4, Color{ 120, 53, 4, 255 });
    
    // Эффект мерцающего огня (динамический радиус на основе sin)
    float f1 = 11.0f + 2.5f * sinf(framesCounter * 0.25f);
    float f2 = 7.0f + 1.8f * sinf(framesCounter * 0.35f + 1.0f);
    float f3 = 4.0f + 1.0f * sinf(framesCounter * 0.45f + 2.0f);
    
    DrawCircleV(position, f1, Color{ 239, 68, 68, 180 });  // Внешнее красное пламя
    DrawCircleV(position, f2, Color{ 245, 158, 11, 210 }); // Среднее оранжевое пламя
    DrawCircleV(position, f3, Color{ 253, 224, 71, 255 }); // Яркое желтое ядро
}

// Рисование палатки для отдыха в лагере
void DrawTent(Vector2 position) {
    // Тень палатки
    DrawTriangle(
        Vector2{ position.x, position.y - 35 },
        Vector2{ position.x - 40, position.y + 12 },
        Vector2{ position.x + 40, position.y + 12 },
        Color{ 68, 43, 26, 255 }
    );
    // Брезентовое покрытие
    DrawTriangle(
        Vector2{ position.x - 4, position.y - 35 },
        Vector2{ position.x - 40, position.y + 12 },
        Vector2{ position.x + 12, position.y + 12 },
        Color{ 180, 83, 9, 255 }
    );
    // Открытый вход в палатку
    DrawTriangle(
        Vector2{ position.x, position.y - 12 },
        Vector2{ position.x - 12, position.y + 12 },
        Vector2{ position.x + 12, position.y + 12 },
        Color{ 24, 24, 27, 255 }
    );
}

// Рисование собираемых предметов (трава, золотоцвет)
void DrawMapItem(MapItem item) {
    if (!item.active) return;
    
    if (item.type == ITEM_HERB) {
        // Зеленая целебная трава с медицинским белым крестиком
        DrawCircleV(item.position, 7, Color{ 34, 197, 94, 140 });
        DrawCircleV(item.position, 4, Color{ 34, 197, 94, 255 });
        DrawRectangle(item.position.x - 1.5f, item.position.y - 5, 3, 10, Color{ 255, 255, 255, 200 });
        DrawRectangle(item.position.x - 5, item.position.y - 1.5f, 10, 3, Color{ 255, 255, 255, 200 });
    } 
    else if (item.type == ITEM_GOLDFLOWER) {
        // Золотой светящийся цветок
        DrawCircleV(item.position, 7, Color{ 245, 158, 11, 110 });
        DrawCircleV(item.position, 4, Color{ 253, 224, 71, 255 });
        DrawCircle(item.position.x, item.position.y, 1.5f, Color{ 255, 255, 255, 255 });
    }
}

// Рисование анимированной блуждающей слизи в мире
void DrawWanderingSlime(WanderingSlime slime, int framesCounter, Color bgDark) {
    if (!slime.active) return;
    
    // Пульсация размеров (эффект прыжка/дыхания)
    float pulse = sinf(framesCounter * 0.14f + slime.id) * 3.5f;
    float rx = 16.0f + pulse * 0.5f;
    float ry = 13.0f - pulse * 0.5f;
    
    Vector2 slimePos = { slime.position.x, slime.position.y + pulse * 0.4f };
    
    // Само тело слизня
    DrawEllipse((int)slimePos.x, (int)slimePos.y, rx, ry, Color{ 34, 197, 94, 180 });
    DrawEllipse((int)slimePos.x, (int)slimePos.y, rx - 3, ry - 3, Color{ 34, 197, 94, 255 });
    
    // Глаза
    DrawCircle(slimePos.x - 5, slimePos.y - 2, 2.5f, Color{ 255, 255, 255, 255 });
    DrawCircle(slimePos.x + 5, slimePos.y - 2, 2.5f, Color{ 255, 255, 255, 255 });
    DrawCircle(slimePos.x - 5, slimePos.y - 2, 1.0f, bgDark);
    DrawCircle(slimePos.x + 5, slimePos.y - 2, 1.0f, bgDark);
}

// ==========================================
// 5. ОСНОВНОЙ КОД ИГРЫ
// ==========================================
int main() {
    // 1. Инициализация окна приложения
    const int windowWidth = 950;
    const int windowHeight = 650;
    InitWindow(windowWidth, windowHeight, "Legends of Green Meadow - 2D Graphical RPG");
    SetTargetFPS(60);

    // 2. Настройка и загрузка кириллического шрифта
    int codepoints[512];
    for (int i = 0; i < 128; i++) codepoints[i] = 32 + i; // ASCII
    for (int i = 0; i < 256; i++) codepoints[128 + i] = 0x0400 + i; // Кириллица
    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, codepoints, 384);
    
    if (font.texture.id == 0) {
        font = GetFontDefault();
    }

    initBiomeRandom();

    // Переменные состояния и персонажа
    GameState state = STATE_WELCOME;
    Player player;
    std::string playerNameInput = "";
    int framesCounter = 0;
    
    // Цветовая палитра интерфейса
    Color bgDark = Color{ 24, 26, 32, 255 };      // Темный фон сцен
    Color bgPanel = Color{ 34, 38, 48, 255 };     // Панели меню
    Color textWhite = Color{ 243, 244, 246, 255 }; // Белый текст
    Color textGray = Color{ 156, 163, 175, 255 };  // Серый текст
    Color btnNormal = Color{ 99, 102, 241, 255 };  // Индиго
    Color btnHover = Color{ 129, 140, 248, 255 };   // Светло-индиго
    Color btnClick = Color{ 79, 70, 229, 255 };    // Темно-индиго
    
    // ==========================================
    // ДВУМЕРНЫЕ ИГРОВЫЕ ПЕРЕМЕННЫЕ
    // ==========================================
    Vector2 playerPos = { 1000.0f, 1000.0f }; // Начальная позиция (в центре лагеря)
    float playerSpeed = 190.0f;
    float playerRadius = 13.0f;
    float playerBobbingY = 0.0f; // Анимация покачивания при ходьбе
    
    // Настройки камеры
    Camera2D camera = { 0 };
    camera.target = playerPos;
    camera.offset = Vector2{ windowWidth / 2.0f, windowHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    // Объекты окружения
    std::vector<Obstacle> obstacles; // Деревья
    std::vector<MapItem> mapItems;   // Травы и цветы
    std::vector<WanderingSlime> slimes; // Слизни
    std::vector<FloatingText> floatingTexts; // Летящий текст урона/золота
    std::vector<std::pair<std::string, float>> miniLog; // Мини-лог на экране мира
    
    // Параметры лагеря
    Vector2 campfirePos = { 960.0f, 1000.0f };
    Vector2 tentPos = { 1040.0f, 980.0f };
    
    // Состояние боевой сцены
    int activeSlimeIndex = -1; // С каким слизнем дерется игрок
    int slimeHp = 0;
    int slimeMaxHp = 0;
    std::string slimeType = "Зеленая Слизь";
    
    // Переменные пошагового боя
    int combatTurnState = 0; // 0 = ход игрока, 1 = ход противника (ожидание), 2 = проигрывание анимации
    float combatTimer = 0.0f;
    std::string combatMessage = "";
    std::vector<std::string> combatLog;
    
    // Визуальные эффекты боя
    float screenShakeIntensity = 0.0f;
    float redFlashTimer = 0.0f;
    float slashEffectTimer = 0.0f;
    
    // ==========================================
    // ЛОГИКА ИНИЦИАЛИЗАЦИИ МИРА
    // ==========================================
    auto InitWorldEntities = [&]() {
        obstacles.clear();
        mapItems.clear();
        slimes.clear();
        floatingTexts.clear();
        miniLog.clear();
        
        playerPos = Vector2{ 1000.0f, 1000.0f };
        camera.target = playerPos;
        
        AddLogMessage("Вы проснулись в безопасном Лагере.", miniLog);
        AddLogMessage("Управление: WASD / Стрелки. Исследуйте Поляну!", miniLog);
        
        // 1. Генерация деревьев (препятствий) вне зоны лагеря
        int attempts = 0;
        while (obstacles.size() < 70 && attempts < 500) {
            attempts++;
            Vector2 candidate = { (float)(50 + rand() % 1900), (float)(50 + rand() % 1900) };
            
            // Запрет спавна в лагере (радиус 220 от центра)
            float distToCamp = sqrtf(powf(candidate.x - 1000.0f, 2) + powf(candidate.y - 1000.0f, 2));
            if (distToCamp < 220.0f) continue;
            
            // Проверка на перекрытие с другими деревьями
            bool overlap = false;
            for (const auto& obs : obstacles) {
                float d = sqrtf(powf(candidate.x - obs.position.x, 2) + powf(candidate.y - obs.position.y, 2));
                if (d < 50.0f) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) {
                obstacles.push_back({ candidate, 14.0f });
            }
        }
        
        // 2. Генерация собираемых ресурсов
        for (int i = 0; i < 16; ++i) {
            // Спавн лечебных трав
            Vector2 pos;
            while (true) {
                pos = { (float)(80 + rand() % 1840), (float)(80 + rand() % 1840) };
                float dist = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                if (dist > 200.0f) break;
            }
            mapItems.push_back({ pos, ITEM_HERB, true, 0.0f });
            
            // Спавн золотоцветов
            while (true) {
                pos = { (float)(80 + rand() % 1840), (float)(80 + rand() % 1840) };
                float dist = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                if (dist > 200.0f) break;
            }
            mapItems.push_back({ pos, ITEM_GOLDFLOWER, true, 0.0f });
        }
        
        // 3. Спавн блуждающих слизней в Meadow
        for (int i = 0; i < 7; ++i) {
            Vector2 pos;
            while (true) {
                pos = { (float)(100 + rand() % 1800), (float)(100 + rand() % 1800) };
                float dist = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                if (dist > 280.0f) break;
            }
            slimes.push_back({ pos, pos, 0.0f, 45.0f + rand() % 25, 25 + rand() % 15, 40, true, i });
            slimes.back().hp = slimes.back().maxHp;
        }
    };

    // Основной цикл приложения
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        framesCounter++;
        
        // --- ОБНОВЛЕНИЕ ИГРОВОЙ ЛОГИКИ (UPDATE) ---
        
        if (state == STATE_WELCOME) {
            // Ввод имени персонажа
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32 && key <= 125) || (key >= 0x0400 && key <= 0x04FF)) {
                    if (playerNameInput.length() < 14) {
                        AppendUnicodeToUTF8(playerNameInput, key);
                    }
                }
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (!playerNameInput.empty()) {
                    do {
                        playerNameInput.pop_back();
                    } while (!playerNameInput.empty() && ((playerNameInput.back() & 0xC0) == 0x80));
                }
            }
        } 
        else if (state == STATE_2D_WORLD) {
            // ==========================================
            // ЛОГИКА ДЛЯ 2D СВОБОДНОГО МИРА
            // ==========================================
            
            // 1. Считывание движения
            Vector2 moveDir = { 0, 0 };
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveDir.y -= 1;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveDir.y += 1;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveDir.x -= 1;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += 1;
            
            bool isMoving = (moveDir.x != 0 || moveDir.y != 0);
            
            if (isMoving) {
                // Нормализация вектора движения
                float len = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
                moveDir.x /= len;
                moveDir.y /= len;
                
                // Анимация покачивания
                playerBobbingY = sinf(framesCounter * 0.2f) * 3.0f;
                
                // Пошаговая проверка коллизий по осям для скольжения вдоль препятствий
                float stepX = moveDir.x * playerSpeed * dt;
                float stepY = moveDir.y * playerSpeed * dt;
                
                // Проверка по оси X
                Vector2 nextPosX = { playerPos.x + stepX, playerPos.y };
                bool colX = false;
                
                // Проверка границ карты
                if (nextPosX.x < 15.0f || nextPosX.x > 1985.0f) colX = true;
                
                // Коллизия с деревьями
                for (const auto& obs : obstacles) {
                    float dist = sqrtf(powf(nextPosX.x - obs.position.x, 2) + powf(nextPosX.y - obs.position.y, 2));
                    if (dist < (playerRadius + obs.radius)) {
                        colX = true;
                        break;
                    }
                }
                if (!colX) playerPos.x = nextPosX.x;
                
                // Проверка по оси Y
                Vector2 nextPosY = { playerPos.x, playerPos.y + stepY };
                bool colY = false;
                
                if (nextPosY.y < 15.0f || nextPosY.y > 1985.0f) colY = true;
                
                for (const auto& obs : obstacles) {
                    float dist = sqrtf(powf(nextPosY.x - obs.position.x, 2) + powf(nextPosY.y - obs.position.y, 2));
                    if (dist < (playerRadius + obs.radius)) {
                        colY = true;
                        break;
                    }
                }
                if (!colY) playerPos.y = nextPosY.y;
            } else {
                playerBobbingY = 0.0f;
            }
            
            // 2. Плавное следование камеры за игроком
            camera.target.x += (playerPos.x - camera.target.x) * 0.1f;
            camera.target.y += (playerPos.y - camera.target.y) * 0.1f;
            
            // 3. Взаимодействие с лагерем (отдых в палатке)
            float distToTent = sqrtf(powf(playerPos.x - tentPos.x, 2) + powf(playerPos.y - tentPos.y, 2));
            if (distToTent < 45.0f && player.health < player.maxHealth) {
                if (IsKeyPressed(KEY_E)) {
                    player.health = player.maxHealth;
                    AddLogMessage("Вы сладко выспались в палатке (+Здоровье восполнено)", miniLog);
                    
                    // Спавн красивого летящего текста над игроком
                    floatingTexts.push_back({ "+HP Восстановлено", Vector2{ playerPos.x, playerPos.y - 15.0f }, Color{ 34, 197, 94, 255 }, 1.0f, -45.0f, 1.2f, true });
                }
            }
            
            // 4. Логика сбора ресурсов
            for (auto& item : mapItems) {
                if (!item.active) {
                    // Таймер респавна предметов
                    item.respawnTimer -= dt;
                    if (item.respawnTimer <= 0.0f) {
                        item.active = true;
                    }
                    continue;
                }
                
                float distToItem = sqrtf(powf(playerPos.x - item.position.x, 2) + powf(playerPos.y - item.position.y, 2));
                if (distToItem < (playerRadius + 8.0f)) {
                    item.active = false;
                    item.respawnTimer = 12.0f; // Перерождение через 12 секунд
                    
                    if (item.type == ITEM_HERB) {
                        // Исцеление
                        int heal = 15 + rand() % 11;
                        int oldHp = player.health;
                        player.health = std::min(player.maxHealth, player.health + heal);
                        int actualHealed = player.health - oldHp;
                        
                        std::stringstream ss;
                        ss << "+" << actualHealed << " HP";
                        floatingTexts.push_back({ ss.str(), Vector2{ item.position.x, item.position.y - 10.0f }, Color{ 34, 197, 94, 255 }, 1.0f, -40.0f, 1.2f, true });
                        
                        std::stringstream ssLog;
                        ssLog << "Вы нашли лечебную траву. Восстановлено " << actualHealed << " HP.";
                        AddLogMessage(ssLog.str(), miniLog);
                    } 
                    else if (item.type == ITEM_GOLDFLOWER) {
                        // Золото
                        int goldLoot = 10 + rand() % 16;
                        player.gold += goldLoot;
                        
                        std::stringstream ss;
                        ss << "+" << goldLoot << " Золота";
                        floatingTexts.push_back({ ss.str(), Vector2{ item.position.x, item.position.y - 10.0f }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.2f, true });
                        
                        std::stringstream ssLog;
                        ssLog << "Собрано золотоцвет: +" << goldLoot << " золотых монет.";
                        AddLogMessage(ssLog.str(), miniLog);
                    }
                }
            }
            
            // 5. Логика движения слизней и коллизий с игроком
            for (size_t i = 0; i < slimes.size(); ++i) {
                auto& slime = slimes[i];
                if (!slime.active) continue;
                
                // Случайное блуждание слизня
                slime.wanderTimer -= dt;
                if (slime.wanderTimer <= 0.0f) {
                    slime.wanderTimer = 2.0f + (rand() % 40) / 10.0f; // Смена направления раз в 2-6 сек
                    
                    // Задаем случайное смещение относительно текущей позиции
                    float angle = (float)(rand() % 360) * (3.14159f / 180.0f);
                    float range = 60.0f + rand() % 120;
                    slime.targetPosition = { slime.position.x + cosf(angle) * range, slime.position.y + sinf(angle) * range };
                    
                    // Ограничения карты и защита от входа в лагерь
                    slime.targetPosition.x = std::max(50.0f, std::min(1950.0f, slime.targetPosition.x));
                    slime.targetPosition.y = std::max(50.0f, std::min(1950.0f, slime.targetPosition.y));
                    
                    float dCamp = sqrtf(powf(slime.targetPosition.x - 1000.0f, 2) + powf(slime.targetPosition.y - 1000.0f, 2));
                    if (dCamp < 250.0f) {
                        // Если целится в лагерь, направляем в противоположную сторону
                        slime.targetPosition = slime.position;
                    }
                }
                
                // Перемещение к целевой позиции
                Vector2 toTarget = { slime.targetPosition.x - slime.position.x, slime.targetPosition.y - slime.position.y };
                float dTarget = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
                if (dTarget > 2.0f) {
                    slime.position.x += (toTarget.x / dTarget) * slime.speed * dt;
                    slime.position.y += (toTarget.y / dTarget) * slime.speed * dt;
                }
                
                // Проверка столкновения с игроком -> Запуск пошагового боя
                float dPlayer = sqrtf(powf(playerPos.x - slime.position.x, 2) + powf(playerPos.y - slime.position.y, 2));
                if (dPlayer < (playerRadius + 15.0f)) {
                    // Переход в режим БИТВЫ
                    state = STATE_MEADOW_SLIME;
                    activeSlimeIndex = i;
                    slimeHp = slime.hp;
                    slimeMaxHp = slime.maxHp;
                    
                    // Выбор типа слизи для разнообразия
                    if (slime.speed > 60.0f) {
                        slimeType = "Быстрая Алая Слизь";
                    } else {
                        slimeType = "Зеленая Слизь";
                    }
                    
                    combatLog.clear();
                    std::stringstream ss;
                    ss << "!!! Вы столкнулись с " << slimeType << "! Начинается пошаговый бой.";
                    combatLog.push_back(ss.str());
                    
                    combatTurnState = 0; // Начинается с хода игрока
                    combatTimer = 0.0f;
                    screenShakeIntensity = 0.0f;
                    redFlashTimer = 0.0f;
                    slashEffectTimer = 0.0f;
                }
            }
            
            // 6. Обновление летящих текстов
            for (auto& txt : floatingTexts) {
                if (!txt.active) continue;
                txt.position.y += txt.ySpeed * dt;
                txt.lifetime -= dt;
                txt.alpha = txt.lifetime / 1.2f; // Плавное угасание альфа-канала
                if (txt.lifetime <= 0.0f) {
                    txt.active = false;
                }
            }
            
            // Обновление таймеров жизни мини-лога
            for (auto& item : miniLog) {
                if (item.second > 0.0f) item.second -= dt;
            }
        } 
        else if (state == STATE_MEADOW_SLIME) {
            // ==========================================
            // ЛОГИКА ПОШАГОВОГО БОЯ
            // ==========================================
            
            // Обновление таймеров спецэффектов экрана
            if (screenShakeIntensity > 0.0f) {
                screenShakeIntensity -= dt * 12.0f;
            }
            if (redFlashTimer > 0.0f) {
                redFlashTimer -= dt;
            }
            if (slashEffectTimer > 0.0f) {
                slashEffectTimer -= dt;
            }
            
            // Логика хода противника (асинхронное поведение с задержкой)
            if (combatTurnState == 1) {
                combatTimer += dt;
                if (combatTimer >= 0.8f) {
                    // Ход слизня: Нанесение случайного урона игроку
                    int slimeDamage = 6 + (rand() % 7); // 6-12 урона
                    
                    // Защита воина снижает урон
                    if (player.className == "Воин") {
                        slimeDamage = std::max(3, slimeDamage - 3);
                    }
                    
                    player.health = std::max(0, player.health - slimeDamage);
                    
                    std::stringstream ss;
                    ss << "Слизь атакует вас на " << slimeDamage << " единиц урона!";
                    combatLog.push_back(ss.str());
                    
                    // Эффекты получения урона
                    redFlashTimer = 0.15f;
                    screenShakeIntensity = 10.0f;
                    
                    // Спавн всплывающего текста над головой игрока
                    std::stringstream ssDmg;
                    ssDmg << "-" << slimeDamage << " HP";
                    floatingTexts.push_back({ ssDmg.str(), Vector2{ windowWidth/4.0f + 10, 220 }, Color{ 239, 68, 68, 255 }, 1.0f, -40.0f, 1.2f, true });
                    
                    if (player.health <= 0) {
                        player.isAlive = false;
                        combatLog.push_back("Вы были повержены в жестоком бою...");
                        combatTurnState = 3; // Конец игры
                        combatTimer = 0.0f;
                    } else {
                        combatTurnState = 0; // Возврат хода игроку
                    }
                    combatTimer = 0.0f;
                }
            }
            // Задержка перед выходом на карту после победы
            else if (combatTurnState == 2) {
                combatTimer += dt;
                if (combatTimer >= 1.4f) {
                    state = STATE_2D_WORLD;
                    combatTimer = 0.0f;
                }
            }
            // Задержка перед экраном Game Over после смерти
            else if (combatTurnState == 3) {
                combatTimer += dt;
                if (combatTimer >= 1.8f) {
                    state = STATE_GAME_OVER;
                    combatTimer = 0.0f;
                }
            }
            
            // Обновление летящих текстов боя
            for (auto& txt : floatingTexts) {
                if (!txt.active) continue;
                txt.position.y += txt.ySpeed * dt;
                txt.lifetime -= dt;
                txt.alpha = txt.lifetime / 1.2f;
                if (txt.lifetime <= 0.0f) {
                    txt.active = false;
                }
            }
        }

        // --- ГРАФИЧЕСКАЯ ОТРИСОВКА (DRAW) ---
        BeginDrawing();
        ClearBackground(bgDark);

        // A. Верхняя панель (Шапка)
        DrawRectangle(0, 0, windowWidth, 95, bgPanel);
        DrawLine(0, 95, windowWidth, 95, Color{ 48, 54, 68, 255 });
        
        if (state == STATE_WELCOME || state == STATE_CLASS_SELECT) {
            DrawTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", Vector2{ 30, 25 }, 28, 1.0f, textWhite);
            DrawTextEx(font, "Двумерная визуальная RPG на Raylib 5.0", Vector2{ 30, 60 }, 14, 1.0f, textGray);
        } 
        else {
            // HUD Статистика Персонажа
            std::string playerTitle = player.name + " (" + player.className + ")";
            DrawTextEx(font, playerTitle.c_str(), Vector2{ 30, 18 }, 20, 1.0f, textWhite);
            
            // Анимированная шкала здоровья HP
            DrawTextEx(font, "HP:", Vector2{ 30, 52 }, 14, 1.0f, textGray);
            DrawProgressBar(Rectangle{ 65, 51, 260, 16 }, (float)player.health, (float)player.maxHealth, Color{ 239, 68, 68, 255 }, Color{ 50, 50, 50, 255 });
            
            std::stringstream ssHP;
            ssHP << player.health << " / " << player.maxHealth;
            DrawTextEx(font, ssHP.str().c_str(), Vector2{ 155, 52 }, 13, 1.0f, textWhite);
            
            // Характеристики
            std::stringstream ssDmg;
            ssDmg << "Урон: " << player.damage;
            DrawTextEx(font, ssDmg.str().c_str(), Vector2{ 370, 36 }, 16, 1.0f, textWhite);
            
            std::stringstream ssGold;
            ssGold << "Золото: " << player.gold;
            DrawTextEx(font, ssGold.str().c_str(), Vector2{ 490, 36 }, 16, 1.0f, Color{ 245, 158, 11, 255 });
            
            // Подсказка управления на экране мира
            if (state == STATE_2D_WORLD) {
                DrawTextEx(font, "Управление: WASD / Стрелки   [E] - Отдохнуть у палатки", Vector2{ windowWidth - 480, 36 }, 14, 1.0f, textGray);
            }
        }

        // B. Отрисовка конкретных экранов в зависимости от GameState
        switch (state) {
            case STATE_WELCOME: {
                // Экран приветствия персонажа
                DrawTextEx(font, "Добро пожаловать в Легенды!", Vector2{ 325, 175 }, 24, 1.0f, textWhite);
                DrawTextEx(font, "Введите имя вашего героя:", Vector2{ 370, 235 }, 15, 1.0f, textGray);
                
                Rectangle inputRect = { 325, 275, 300, 45 };
                DrawRectangleRounded(inputRect, 0.2f, 4, bgPanel);
                DrawRectangleRoundedLines(inputRect, 0.2f, 4, 2.0f, btnNormal);
                
                DrawTextEx(font, playerNameInput.c_str(), Vector2{ inputRect.x + 15, inputRect.y + 11 }, 20, 1.0f, textWhite);
                
                // Анимация мерцания курсора
                if (((framesCounter / 30) % 2) == 0) {
                    float cursorX = inputRect.x + 15 + MeasureTextEx(font, playerNameInput.c_str(), 20, 1.0f).x;
                    DrawRectangle((int)cursorX + 2, (int)inputRect.y + 11, 2, 22, textWhite);
                }
                
                bool nameIsValid = !playerNameInput.empty();
                Color cBtn = nameIsValid ? btnNormal : Color{ 70, 75, 90, 255 };
                Color cHover = nameIsValid ? btnHover : Color{ 70, 75, 90, 255 };
                Color cText = nameIsValid ? textWhite : textGray;
                
                if (DrawButton(font, Rectangle{ 375, 355, 200, 45 }, "Продолжить", cBtn, cHover, btnClick, cText)) {
                    if (nameIsValid) {
                        player.name = playerNameInput;
                        state = STATE_CLASS_SELECT;
                    }
                }
                break;
            }
            case STATE_CLASS_SELECT: {
                // Карточки выбора класса
                DrawTextEx(font, "Выберите класс вашего персонажа:", Vector2{ 310, 130 }, 22, 1.0f, textWhite);
                
                // 1. Воин
                if (DrawClassCard(font, Rectangle{ 60, 190, 250, 350 }, "Воин", "HP: 120   DMG: 10", 
                    "Сила и выносливость - твое всё!\nОтличный выбор для тех, кто предпочитает надежную защиту и бой лоб в лоб.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                    player.className = "Воин";
                    player.maxHealth = 120;
                    player.health = 120;
                    player.damage = 10;
                    player.gold = 100;
                    player.isAlive = true;
                    InitWorldEntities();
                    state = STATE_2D_WORLD;
                }
                
                // 2. Маг
                if (DrawClassCard(font, Rectangle{ 350, 190, 250, 350 }, "Маг", "HP: 80    DMG: 20", 
                    "Тайная магия сокрушит твоих врагов!\nНаносит огромный урон, но обладает хрупким здоровьем.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                    player.className = "Маг";
                    player.maxHealth = 80;
                    player.health = 80;
                    player.damage = 20;
                    player.gold = 100;
                    player.isAlive = true;
                    InitWorldEntities();
                    state = STATE_2D_WORLD;
                }
                
                // 3. Лучник
                if (DrawClassCard(font, Rectangle{ 640, 190, 250, 350 }, "Лучник", "HP: 100   DMG: 15", 
                    "Меткость и скорость - твои козыри!\nСбалансированный быстрый боец, готовый к любым условиям.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                    player.className = "Лучник";
                    player.maxHealth = 100;
                    player.health = 100;
                    player.damage = 15;
                    player.gold = 100;
                    player.isAlive = true;
                    InitWorldEntities();
                    state = STATE_2D_WORLD;
                }
                break;
            }
            case STATE_2D_WORLD: {
                // ==========================================
                // ОТРИСОВКА ВИЗУАЛЬНОГО ДВУМЕРНОГО МИРА
                // ==========================================
                
                BeginMode2D(camera);
                
                // 1. Отрисовка травы (сетка 2000x2000)
                // Вместо тяжелого рендеринга нарисуем фоновый квадрат
                DrawRectangle(0, 0, 2000, 2000, Color{ 22, 101, 52, 120 }); // Темно-зеленая земля
                
                // Тонкие линии координатной сетки для ощущения глубины
                for (int x = 0; x <= 2000; x += 100) {
                    DrawLine(x, 0, x, 2000, Color{ 20, 80, 40, 60 });
                }
                for (int y = 0; y <= 2000; y += 100) {
                    DrawLine(0, y, 2000, y, Color{ 20, 80, 40, 60 });
                }
                
                // Рисование декоративной мелкой травы на карте
                srand(12345); // Фиксированный сид для статичной прорисовки
                for (int i = 0; i < 80; ++i) {
                    int tx = rand() % 2000;
                    int ty = rand() % 2000;
                    DrawLine(tx, ty, tx + 2, ty - 6, Color{ 34, 197, 94, 90 });
                    DrawLine(tx + 2, ty - 6, tx + 4, ty, Color{ 34, 197, 94, 90 });
                }
                
                // 2. Отрисовка Лагеря (ковер из старых досок/камня)
                DrawCircle(1000, 1000, 150, Color{ 63, 63, 70, 255 }); // Земляная брусчатка лагеря
                DrawCircleLines(1000, 1000, 150, Color{ 82, 82, 91, 255 });
                
                // Палатка и костер
                DrawTent(tentPos);
                DrawCampfire(campfirePos, framesCounter);
                
                // Подсказка у палатки
                float distToTent = sqrtf(powf(playerPos.x - tentPos.x, 2) + powf(playerPos.y - tentPos.y, 2));
                if (distToTent < 45.0f && player.health < player.maxHealth) {
                    DrawRectangle(tentPos.x - 70, tentPos.y - 65, 140, 25, Color{ 30, 30, 30, 200 });
                    DrawTextEx(font, "[E] ОТДОХНУТЬ", Vector2{ tentPos.x - 55, tentPos.y - 60 }, 13, 1.0f, Color{ 253, 224, 71, 255 });
                }
                
                // 3. Рисование собираемых предметов на поляне
                for (const auto& item : mapItems) {
                    DrawMapItem(item);
                }
                
                // 4. Отрисовка блуждающих слизней
                for (const auto& slime : slimes) {
                    DrawWanderingSlime(slime, framesCounter, bgDark);
                }
                
                // 5. Отрисовка препятствий (деревья)
                // Сортировка по Y для имитации 2.5D глубины
                for (const auto& obs : obstacles) {
                    // Рисуем тень дерева
                    DrawEllipse(obs.position.x, obs.position.y + 25, 12, 6, Color{ 0, 0, 0, 100 });
                    DrawTree(obs.position, obs.radius);
                }
                
                // 6. Отрисовка игрока (с тенью под ногами)
                DrawEllipse(playerPos.x, playerPos.y + 12, 10, 5, Color{ 0, 0, 0, 120 });
                DrawPlayerAvatar(player.className, playerPos, playerRadius, playerBobbingY, bgDark);
                
                // 7. Отрисовка летящих текстов
                for (const auto& txt : floatingTexts) {
                    if (!txt.active) continue;
                    DrawTextEx(font, txt.text.c_str(), txt.position, 16, 1.0f, ColorAlpha(txt.color, txt.alpha));
                }
                
                // Отрисовка невидимых границ карты
                DrawRectangleLines(0, 0, 2000, 2000, Color{ 220, 38, 38, 100 });
                
                EndMode2D();
                
                // --- ОТРИСОВКА ИНТЕРФЕЙСА ПОВЕРХ 2D СЦЕНЫ (HUD) ---
                
                // Угасающий мини-журнал в левом нижнем углу экрана
                int logY = windowHeight - 40;
                for (int i = (int)miniLog.size() - 1; i >= 0; --i) {
                    float life = miniLog[i].second;
                    if (life <= 0.0f) continue;
                    
                    float alpha = std::min(1.0f, life);
                    Color col = ColorAlpha(textWhite, alpha);
                    
                    if (miniLog[i].first.find("+HP") != std::string::npos) {
                        col = ColorAlpha(Color{ 34, 197, 94, 255 }, alpha);
                    } else if (miniLog[i].first.find("золото") != std::string::npos) {
                        col = ColorAlpha(Color{ 245, 158, 11, 255 }, alpha);
                    }
                    
                    // Темная плашка под текст лога
                    Vector2 txtSize = MeasureTextEx(font, miniLog[i].first.c_str(), 14, 1.0f);
                    DrawRectangle(20, logY - 2, txtSize.x + 20, 20, ColorAlpha(bgDark, 180 * alpha));
                    
                    DrawTextEx(font, miniLog[i].first.c_str(), Vector2{ 30, (float)logY }, 14, 1.0f, col);
                    logY -= 24;
                }
                break;
            }
            case STATE_MEADOW_SLIME: {
                // ==========================================
                // ОТРИСОВКА ГРАФИЧЕСКОЙ СЦЕНЫ БОЯ
                // ==========================================
                
                // 1. Применяем эффект тряски экрана (Screen Shake) при ударах
                Vector2 shakeOffset = { 0, 0 };
                if (screenShakeIntensity > 0.0f) {
                    shakeOffset.x = (float)(rand() % 20 - 10) * (screenShakeIntensity / 10.0f);
                    shakeOffset.y = (float)(rand() % 20 - 10) * (screenShakeIntensity / 10.0f);
                }
                
                // Подложка арены боя
                DrawRectangle(30 + shakeOffset.x, 120 + shakeOffset.y, windowWidth - 60, 490, bgPanel);
                DrawRectangleLines(30 + shakeOffset.x, 120 + shakeOffset.y, windowWidth - 60, 490, Color{ 48, 54, 68, 255 });
                
                // Левая зона: Карточка игрока
                Rectangle pCard = { 80 + shakeOffset.x, 170 + shakeOffset.y, 240, 280 };
                DrawRectangleRounded(pCard, 0.1f, 4, Color{ 26, 28, 35, 255 });
                DrawRectangleRoundedLines(pCard, 0.1f, 4, 1.5f, btnNormal);
                
                DrawPlayerAvatar(player.className, Vector2{ pCard.x + pCard.width/2.0f, pCard.y + 80 }, 45.0f, 0.0f, bgDark);
                
                std::string pName = player.name + " (" + player.className + ")";
                Vector2 pNameSize = MeasureTextEx(font, pName.c_str(), 18, 1.0f);
                DrawTextEx(font, pName.c_str(), Vector2{ pCard.x + (pCard.width - pNameSize.x)/2.0f, pCard.y + 150 }, 18, 1.0f, textWhite);
                
                std::stringstream ssStats;
                ssStats << "Урон: " << player.damage << "   Золото: " << player.gold;
                Vector2 pStatsSize = MeasureTextEx(font, ssStats.str().c_str(), 14, 1.0f);
                DrawTextEx(font, ssStats.str().c_str(), Vector2{ pCard.x + (pCard.width - pStatsSize.x)/2.0f, pCard.y + 185 }, 14, 1.0f, textGray);
                
                DrawProgressBar(Rectangle{ pCard.x + 20, pCard.y + 225, 200, 16 }, (float)player.health, (float)player.maxHealth, Color{ 239, 68, 68, 255 }, Color{ 50, 50, 50, 255 });
                std::stringstream ssPlayerHp;
                ssPlayerHp << player.health << " / " << player.maxHealth << " HP";
                Vector2 pHPSize = MeasureTextEx(font, ssPlayerHp.str().c_str(), 12, 1.0f);
                DrawTextEx(font, ssPlayerHp.str().c_str(), Vector2{ pCard.x + (pCard.width - pHPSize.x)/2.0f, pCard.y + 248 }, 12, 1.0f, textWhite);
                
                // Центральный разделитель (VERSUS)
                DrawTextEx(font, "VS", Vector2{ windowWidth/2.0f - 15 + shakeOffset.x, 280 + shakeOffset.y }, 26, 1.0f, Color{ 239, 68, 68, 180 });
                
                // Правая зона: Карточка Слизня
                Rectangle sCard = { windowWidth - 320 + shakeOffset.x, 170 + shakeOffset.y, 240, 280 };
                DrawRectangleRounded(sCard, 0.1f, 4, Color{ 26, 28, 35, 255 });
                DrawRectangleRoundedLines(sCard, 0.1f, 4, 1.5f, Color{ 239, 68, 68, 255 });
                
                // Отрисовка гигантской милой слизи
                float combatSlimeBobbing = sinf(framesCounter * 0.16f) * 6.0f;
                float sRadX = 55.0f + combatSlimeBobbing * 0.7f;
                float sRadY = 45.0f - combatSlimeBobbing * 0.7f;
                Vector2 slimeCenter = { sCard.x + sCard.width/2.0f, sCard.y + 80 + combatSlimeBobbing * 0.5f };
                
                // Специфический цвет для разных типов слизи
                Color slimeColor = Color{ 34, 197, 94, 255 }; // Зеленая
                if (slimeType.find("Алая") != std::string::npos) {
                    slimeColor = Color{ 220, 38, 38, 255 }; // Алая слизь
                }
                
                if (slimeHp > 0) {
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y, sRadX, sRadY, ColorAlpha(slimeColor, 0.7f));
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y, sRadX - 4, sRadY - 4, slimeColor);
                    
                    // Большие круглые глаза
                    DrawCircle(slimeCenter.x - 14, slimeCenter.y - 6, 6, textWhite);
                    DrawCircle(slimeCenter.x + 14, slimeCenter.y - 6, 6, textWhite);
                    DrawCircle(slimeCenter.x - 14, slimeCenter.y - 6, 3, bgDark);
                    DrawCircle(slimeCenter.x + 14, slimeCenter.y - 6, 3, bgDark);
                } else {
                    // Эффект таяния мертвой слизи
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y + 30, sRadX + 15, 8, ColorAlpha(slimeColor, 120));
                }
                
                Vector2 sNameSize = MeasureTextEx(font, slimeType.c_str(), 18, 1.0f);
                DrawTextEx(font, slimeType.c_str(), Vector2{ sCard.x + (sCard.width - sNameSize.x)/2.0f, sCard.y + 150 }, 18, 1.0f, textWhite);
                
                // Здоровье слизня
                if (slimeHp > 0) {
                    DrawProgressBar(Rectangle{ sCard.x + 20, sCard.y + 225, 200, 16 }, (float)slimeHp, (float)slimeMaxHp, Color{ 34, 197, 94, 255 }, Color{ 50, 50, 50, 255 });
                    std::stringstream ssSlimeHp;
                    ssSlimeHp << slimeHp << " / " << slimeMaxHp << " HP";
                    Vector2 sHPSize = MeasureTextEx(font, ssSlimeHp.str().c_str(), 12, 1.0f);
                    DrawTextEx(font, ssSlimeHp.str().c_str(), Vector2{ sCard.x + (sCard.width - sHPSize.x)/2.0f, sCard.y + 248 }, 12, 1.0f, textWhite);
                } else {
                    DrawTextEx(font, "ПОБЕЖДЕН", Vector2{ sCard.x + 75, sCard.y + 225 }, 16, 1.0f, Color{ 34, 197, 94, 255 });
                }
                
                // Отрисовка спецэффекта удара (Sharp Slash Line)
                if (slashEffectTimer > 0.0f) {
                    DrawLineEx(
                        Vector2{ slimeCenter.x - 60, slimeCenter.y - 60 },
                        Vector2{ slimeCenter.x + 60, slimeCenter.y + 60 },
                        6.0f,
                        Color{ 255, 255, 255, 220 }
                    );
                    DrawLineEx(
                        Vector2{ slimeCenter.x - 60, slimeCenter.y - 60 },
                        Vector2{ slimeCenter.x + 60, slimeCenter.y + 60 },
                        2.0f,
                        Color{ 239, 68, 68, 255 }
                    );
                }
                
                // Отрисовка летящих текстов урона/лечения на арене боя
                for (const auto& txt : floatingTexts) {
                    if (!txt.active) continue;
                    DrawTextEx(font, txt.text.c_str(), txt.position, 22, 1.0f, ColorAlpha(txt.color, txt.alpha));
                }
                
                // Красная вспышка при получении урона
                if (redFlashTimer > 0.0f) {
                    DrawRectangle(30, 120, windowWidth - 60, 490, Color{ 239, 68, 68, (unsigned char)(90 * (redFlashTimer / 0.15f)) });
                }
                
                // Панель управления боем внизу
                Rectangle combatCtrlRect = { 80 + shakeOffset.x, 470 + shakeOffset.y, windowWidth - 160, 120 };
                DrawRectangleRounded(combatCtrlRect, 0.15f, 4, Color{ 26, 28, 35, 255 });
                DrawRectangleRoundedLines(combatCtrlRect, 0.15f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                if (slimeHp > 0 && player.health > 0) {
                    if (combatTurnState == 0) {
                        // Кнопки действий игрока
                        if (DrawButton(font, Rectangle{ combatCtrlRect.x + 30, combatCtrlRect.y + 35, 220, 50 }, "Атаковать Оружием", Color{ 239, 68, 68, 255 }, Color{ 248, 113, 113, 255 }, Color{ 185, 28, 28, 255 }, textWhite)) {
                            // Игрок бьет слизь
                            int damageDealt = player.damage;
                            
                            // Маг наносит критический удар с шансом 30%
                            if (player.className == "Маг") {
                                if (rand() % 100 < 30) {
                                    damageDealt *= 1.5f;
                                    combatLog.push_back("КРИТИЧЕСКОЕ ЗАКЛИНАНИЕ!");
                                }
                            }
                            
                            slimeHp = std::max(0, slimeHp - damageDealt);
                            
                            std::stringstream ss;
                            ss << "Вы атаковали слизь и нанесли " << damageDealt << " урона!";
                            combatLog.push_back(ss.str());
                            
                            // Запуск эффектов
                            slashEffectTimer = 0.2f;
                            screenShakeIntensity = 7.0f;
                            
                            std::stringstream ssDmg;
                            ssDmg << "-" << damageDealt;
                            floatingTexts.push_back({ ssDmg.str(), Vector2{ slimeCenter.x, slimeCenter.y - 30 }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.2f, true });
                            
                            if (slimeHp <= 0) {
                                // Победа над слизнем
                                int goldReward = 15 + rand() % 16;
                                player.gold += goldReward;
                                
                                std::stringstream ssWin;
                                ssWin << "Вы победили слизь и нашли монеты (+" << goldReward << " золота)!";
                                combatLog.push_back(ssWin.str());
                                
                                // Пометка слизня как убитого в мире
                                if (activeSlimeIndex != -1) {
                                    slimes[activeSlimeIndex].active = false;
                                }
                                
                                std::stringstream ssGoldFloat;
                                ssGoldFloat << "+" << goldReward << " Золота";
                                floatingTexts.push_back({ ssGoldFloat.str(), Vector2{ slimeCenter.x, slimeCenter.y - 10 }, Color{ 253, 224, 71, 255 }, 1.0f, -45.0f, 1.4f, true });
                                
                                combatTurnState = 2; // Победа, задержка перед выходом
                                combatTimer = 0.0f;
                            } else {
                                combatTurnState = 1; // Ход противника
                                combatTimer = 0.0f;
                            }
                        }
                        
                        if (DrawButton(font, Rectangle{ combatCtrlRect.x + 280, combatCtrlRect.y + 35, 220, 50 }, "Попытаться приручить", Color{ 16, 185, 129, 255 }, Color{ 52, 211, 153, 255 }, Color{ 4, 120, 87, 255 }, textWhite)) {
                            // Приручение на основе формулы из biom.cc
                            std::string res = handleSlimeBefriend(player);
                            combatLog.push_back(res);
                            
                            if (activeSlimeIndex != -1) {
                                slimes[activeSlimeIndex].active = false; // Слизь исчезает в любом случае
                            }
                            
                            if (!player.isAlive) {
                                // 20% летальный исход
                                combatTurnState = 3;
                                combatTimer = 0.0f;
                            } else {
                                // 80% успех, получаем баффы
                                floatingTexts.push_back({ "УСПЕХ! СИЛА +", Vector2{ pCard.x + 40, pCard.y - 20 }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.5f, true });
                                combatTurnState = 2;
                                combatTimer = 0.0f;
                            }
                        }
                        
                        if (DrawButton(font, Rectangle{ combatCtrlRect.x + 530, combatCtrlRect.y + 35, 220, 50 }, "Сбежать в страхе", Color{ 107, 114, 128, 255 }, Color{ 156, 163, 175, 255 }, Color{ 75, 85, 99, 255 }, textWhite)) {
                            // Побег
                            std::string res = handleSlimeRun(player);
                            combatLog.push_back(res);
                            
                            // Даем игроку 2 сек неуязвимости в мире, сдвигаем слизня чуть назад
                            if (activeSlimeIndex != -1) {
                                float angle = (float)(rand() % 360) * (3.14159f / 180.0f);
                                slimes[activeSlimeIndex].position.x += cosf(angle) * 70.0f;
                                slimes[activeSlimeIndex].position.y += sinf(angle) * 70.0f;
                            }
                            
                            combatTurnState = 2;
                            combatTimer = 0.0f;
                        }
                    } else {
                        // Отрисовка текущего лога боя в окне контроллера (когда идет ход соперника)
                        std::string activeText = combatLog.back();
                        Vector2 textSize = MeasureTextEx(font, activeText.c_str(), 16, 1.0f);
                        DrawTextEx(font, activeText.c_str(), Vector2{ combatCtrlRect.x + (combatCtrlRect.width - textSize.x)/2.0f, combatCtrlRect.y + 48 }, 16, 1.0f, textWhite);
                    }
                } else {
                    // Отрисовка результатов по завершению битвы
                    std::string endText = combatLog.back();
                    Vector2 textSize = MeasureTextEx(font, endText.c_str(), 16, 1.0f);
                    DrawTextEx(font, endText.c_str(), Vector2{ combatCtrlRect.x + (combatCtrlRect.width - textSize.x)/2.0f, combatCtrlRect.y + 48 }, 16, 1.0f, textWhite);
                }
                break;
            }
            case STATE_GAME_OVER: {
                // Экран поражения игрока
                DrawTextEx(font, "ГЕРОЙ ПАЛ В БОЮ", Vector2{ 350, 180 }, 32, 1.0f, Color{ 239, 68, 68, 255 });
                DrawTextEx(font, "Ваше путешествие завершилось трагической гибелью...", Vector2{ 275, 240 }, 16, 1.0f, textGray);
                
                Rectangle statsRect = { 325, 290, 300, 135 };
                DrawRectangleRounded(statsRect, 0.1f, 4, bgPanel);
                DrawRectangleRoundedLines(statsRect, 0.1f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                std::string fn = "Имя: " + player.name;
                DrawTextEx(font, fn.c_str(), Vector2{ statsRect.x + 20, statsRect.y + 20 }, 16, 1.0f, textWhite);
                
                std::string fc = "Класс: " + player.className;
                DrawTextEx(font, fc.c_str(), Vector2{ statsRect.x + 20, statsRect.y + 50 }, 16, 1.0f, textWhite);
                
                std::stringstream ssG;
                ssG << "Собрано золота: " << player.gold;
                DrawTextEx(font, ssG.str().c_str(), Vector2{ statsRect.x + 20, statsRect.y + 80 }, 16, 1.0f, Color{ 245, 158, 11, 255 });
                
                if (DrawButton(font, Rectangle{ 375, 465, 200, 45 }, "Начать сначала", btnNormal, btnHover, btnClick, textWhite)) {
                    playerNameInput = "";
                    state = STATE_WELCOME;
                }
                break;
            }
        }

        EndDrawing();
    }

    // Очистка ресурсов
    UnloadFont(font);
    CloseWindow();
    return 0;
}
