#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "raylib.h"
#include "player.h"
#include "biom.h"

// Перечисление игровых состояний (State Machine)
enum GameState {
    STATE_WELCOME,         // Ввод имени персонажа
    STATE_CLASS_SELECT,    // Выбор класса
    STATE_CAMP,            // Главный лагерь
    STATE_MEADOW,          // Исследование Поляны
    STATE_MEADOW_SLIME,    // Встреча со Слизью
    STATE_GAME_OVER        // Герой погиб
};

// Функция для кодирования юникод-кодов в UTF-8
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

// Вспомогательная функция для добавления сообщений в лог событий
void AddLog(const std::string& msg, std::vector<std::string>& log) {
    log.push_back(msg);
    if (log.size() > 100) {
        log.erase(log.begin());
    }
}

// Отрисовка текста с автопереносом строк (поддерживает UTF-8)
void DrawWrappedText(Font font, const char* text, int posX, int posY, int maxWidth, int fontSize, Color color) {
    std::string textStr(text);
    std::vector<std::string> words;
    std::string currentWord = "";
    
    // Разбиение текста на слова с учетом символов новой строки
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

// Отрисовка кнопки с визуальным откликом при наведении и нажатии
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

// Отрисовка прогресс-бара HP персонажа
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

// Отрисовка карточки класса персонажа при создании
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
    
    // Название класса
    DrawTextEx(font, title, Vector2{ rect.x + 20, rect.y + 20 }, 22.0f, 1.0f, Color{ 243, 244, 246, 255 });
    
    // Характеристики
    DrawTextEx(font, stats, Vector2{ rect.x + 20, rect.y + 55 }, 16.0f, 1.0f, Color{ 245, 158, 11, 255 });
    
    // Описание
    DrawWrappedText(font, desc, rect.x + 20, rect.y + 90, rect.width - 40, 14, Color{ 156, 163, 175, 255 });
    
    return clicked;
}

int main() {
    // 1. Инициализация окна Raylib
    const int windowWidth = 950;
    const int windowHeight = 650;
    InitWindow(windowWidth, windowHeight, "Legends of Green Meadow - RPG");
    SetTargetFPS(60);

    // 2. Загрузка шрифта Arial с поддержкой кириллицы (стандартный шрифт Windows)
    int codepoints[512];
    for (int i = 0; i < 128; i++) codepoints[i] = 32 + i; // ASCII
    for (int i = 0; i < 256; i++) codepoints[128 + i] = 0x0400 + i; // Кириллица
    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, codepoints, 384);
    
    // Если шрифт не загрузился, используем стандартный шрифт Raylib
    if (font.texture.id == 0) {
        font = GetFontDefault();
    }

    // Инициализация генератора случайных чисел для биомов
    initBiomeRandom();

    // 3. Состояние игрового цикла и переменные
    GameState state = STATE_WELCOME;
    Player player;
    std::string playerNameInput = "";
    int framesCounter = 0; // Для анимации мигания курсора ввода имени
    
    // Цвета интерфейса
    Color bgDark = Color{ 26, 28, 35, 255 };      // Фон приложения
    Color bgPanel = Color{ 36, 40, 50, 255 };     // Фон панелей
    Color textWhite = Color{ 243, 244, 246, 255 }; // Белый текст
    Color textGray = Color{ 156, 163, 175, 255 };  // Серый текст
    Color btnNormal = Color{ 99, 102, 241, 255 };  // Кнопка
    Color btnHover = Color{ 129, 140, 248, 255 };   // Кнопка (наведение)
    Color btnClick = Color{ 79, 70, 229, 255 };    // Кнопка (клик)
    
    std::vector<std::string> eventLog;
    AddLog("Добро пожаловать в Легенды Зеленой Поляны!", eventLog);

    // 4. Основной цикл приложения
    while (!WindowShouldClose()) {
        framesCounter++;
        
        // --- ОБРАБОТКА ВВОДА ДЛЯ КАЖДОГО СОСТОЯНИЯ ---
        if (state == STATE_WELCOME) {
            // Ввод имени персонажа
            int key = GetCharPressed();
            while (key > 0) {
                // Разрешаем буквы, цифры и пробелы (как латиницу, так и кириллицу)
                if ((key >= 32 && key <= 125) || (key >= 0x0400 && key <= 0x04FF)) {
                    if (playerNameInput.length() < 15) {
                        AppendUnicodeToUTF8(playerNameInput, key);
                    }
                }
                key = GetCharPressed();
            }
            
            // Удаление последнего символа (Backspace)
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (!playerNameInput.empty()) {
                    // Удаляем целый UTF-8 символ (байт-последовательность)
                    do {
                        playerNameInput.pop_back();
                    } while (!playerNameInput.empty() && ((playerNameInput.back() & 0xC0) == 0x80));
                }
            }
        }

        // --- DRAWING (Отрисовка) ---
        BeginDrawing();
        ClearBackground(bgDark);

        // A. Отрисовка Шапки (Header Panel)
        DrawRectangle(0, 0, windowWidth, 100, bgPanel);
        DrawLine(0, 100, windowWidth, 100, Color{ 48, 54, 68, 255 });
        
        if (state == STATE_WELCOME || state == STATE_CLASS_SELECT) {
            // Заголовок игры
            DrawTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", Vector2{ 30, 30 }, 28, 1.0f, textWhite);
            DrawTextEx(font, "Текстовая RPG с графическим интерфейсом", Vector2{ 30, 65 }, 14, 1.0f, textGray);
        } else {
            // Панель персонажа в процессе игры
            std::string playerTitle = player.name + " (" + player.className + ")";
            DrawTextEx(font, playerTitle.c_str(), Vector2{ 30, 20 }, 22, 1.0f, textWhite);
            
            // HP Бар
            DrawTextEx(font, "Здоровье:", Vector2{ 30, 52 }, 14, 1.0f, textGray);
            DrawProgressBar(Rectangle{ 110, 52, 250, 18 }, (float)player.health, (float)player.maxHealth, Color{ 239, 68, 68, 255 }, Color{ 50, 50, 50, 255 });
            
            std::stringstream ssHP;
            ssHP << player.health << " / " << player.maxHealth << " HP";
            DrawTextEx(font, ssHP.str().c_str(), Vector2{ 180, 53 }, 13, 1.0f, textWhite);
            
            // Характеристики
            std::stringstream ssDmg;
            ssDmg << "Урон: " << player.damage;
            DrawTextEx(font, ssDmg.str().c_str(), Vector2{ 390, 40 }, 18, 1.0f, textWhite);
            
            std::stringstream ssGold;
            ssGold << "Золото: " << player.gold;
            DrawTextEx(font, ssGold.str().c_str(), Vector2{ 520, 40 }, 18, 1.0f, Color{ 245, 158, 11, 255 });
        }

        // B. Отрисовка конкретных экранов в зависимости от GameState
        switch (state) {
            case STATE_WELCOME: {
                // Экран приветствия и ввода имени
                DrawTextEx(font, "Добро пожаловать, Авантюрист!", Vector2{ 320, 180 }, 24, 1.0f, textWhite);
                DrawTextEx(font, "Введите имя вашего героя:", Vector2{ 370, 240 }, 16, 1.0f, textGray);
                
                // Рамка поля ввода имени
                Rectangle inputRect = { 325, 280, 300, 45 };
                DrawRectangleRounded(inputRect, 0.2f, 4, bgPanel);
                DrawRectangleRoundedLines(inputRect, 0.2f, 4, 2.0f, btnNormal);
                
                // Имя
                DrawTextEx(font, playerNameInput.c_str(), Vector2{ inputRect.x + 15, inputRect.y + 11 }, 20, 1.0f, textWhite);
                
                // Курсор
                if (((framesCounter / 30) % 2) == 0) {
                    float cursorX = inputRect.x + 15 + MeasureTextEx(font, playerNameInput.c_str(), 20, 1.0f).x;
                    DrawRectangle((int)cursorX + 2, (int)inputRect.y + 11, 2, 22, textWhite);
                }
                
                // Кнопка продолжения
                bool nameIsValid = !playerNameInput.empty();
                Color cBtn = nameIsValid ? btnNormal : Color{ 70, 75, 90, 255 };
                Color cHover = nameIsValid ? btnHover : Color{ 70, 75, 90, 255 };
                Color cText = nameIsValid ? textWhite : textGray;
                
                if (DrawButton(font, Rectangle{ 375, 360, 200, 45 }, "Продолжить", cBtn, cHover, btnClick, cText)) {
                    if (nameIsValid) {
                        player.name = playerNameInput;
                        state = STATE_CLASS_SELECT;
                    }
                }
                break;
            }
            case STATE_CLASS_SELECT: {
                // Выбор класса
                DrawTextEx(font, "Выберите класс вашего персонажа:", Vector2{ 300, 130 }, 22, 1.0f, textWhite);
                
                // 1. Воин
                if (DrawClassCard(font, Rectangle{ 60, 190, 250, 350 }, "Воин", "HP: 120   DMG: 10", 
                    "Сила и выносливость - твое всё!\nОтличный выбор для тех, кто предпочитает надежную защиту.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                    player.className = "Воин";
                    player.maxHealth = 120;
                    player.health = 120;
                    player.damage = 10;
                    player.gold = 100;
                    player.isAlive = true;
                    AddLog("Вы выбрали класс Воин. Да начнется путешествие!", eventLog);
                    state = STATE_CAMP;
                }
                
                // 2. Маг
                if (DrawClassCard(font, Rectangle{ 350, 190, 250, 350 }, "Маг", "HP: 80    DMG: 20", 
                    "Тайная магия сокрушит твоих врагов!\nНаносит сокрушительный урон, но обладает меньшим запасом сил.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                    player.className = "Маг";
                    player.maxHealth = 80;
                    player.health = 80;
                    player.damage = 20;
                    player.gold = 100;
                    player.isAlive = true;
                    AddLog("Вы выбрали класс Маг. Да пребудет с вами магия!", eventLog);
                    state = STATE_CAMP;
                }
                
                // 3. Лучник
                if (DrawClassCard(font, Rectangle{ 640, 190, 250, 350 }, "Лучник", "HP: 100   DMG: 15", 
                    "Меткость и скорость - твои козыри!\nСбалансированный боец, способный быстро реагировать в любой ситуации.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                    player.className = "Лучник";
                    player.maxHealth = 100;
                    player.health = 100;
                    player.damage = 15;
                    player.gold = 100;
                    player.isAlive = true;
                    AddLog("Вы выбрали класс Лучник. Меткого выстрела!", eventLog);
                    state = STATE_CAMP;
                }
                break;
            }
            case STATE_CAMP:
            case STATE_MEADOW:
            case STATE_MEADOW_SLIME: {
                // Основной игровой экран: Разделение на левую (действия) и правую (журнал) зоны
                
                // 1. Левая панель действий
                Rectangle actPanel = { 30, 120, 440, 500 };
                DrawRectangleRounded(actPanel, 0.05f, 4, bgPanel);
                DrawRectangleRoundedLines(actPanel, 0.05f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                std::string currentArea = (state == STATE_CAMP) ? "БЕЗОПАСНЫЙ ЛАГЕРЬ" : "ЗЕЛЕНАЯ ПОЛЯНА";
                DrawTextEx(font, currentArea.c_str(), Vector2{ 50, 140 }, 20, 1.0f, Color{ 245, 158, 11, 255 });
                DrawLine(50, 170, 450, 170, Color{ 48, 54, 68, 255 });
                
                // Различные кнопки действий
                if (state == STATE_CAMP) {
                    // Кнопки в Лагере
                    if (DrawButton(font, Rectangle{ 60, 200, 380, 50 }, "Отправиться на Зеленую Поляну", btnNormal, btnHover, btnClick, textWhite)) {
                        AddLog("Вы вышли из лагеря и направились на Зеленую Поляну.", eventLog);
                        state = STATE_MEADOW;
                    }
                    
                    if (DrawButton(font, Rectangle{ 60, 280, 380, 50 }, "Завершить игру и выйти", Color{ 239, 68, 68, 220 }, Color{ 248, 113, 113, 255 }, Color{ 185, 28, 28, 255 }, textWhite)) {
                        CloseWindow();
                        return 0;
                    }
                } 
                else if (state == STATE_MEADOW) {
                    // Кнопки на Поляне
                    if (DrawButton(font, Rectangle{ 60, 200, 380, 45 }, "Осмотреться вокруг", btnNormal, btnHover, btnClick, textWhite)) {
                        AddLog(exploreMeadowLookAround(), eventLog);
                    }
                    
                    if (DrawButton(font, Rectangle{ 60, 260, 380, 45 }, "Собирать лечебные травы и цветы", btnNormal, btnHover, btnClick, textWhite)) {
                        AddLog(exploreMeadowGatherHerbs(player), eventLog);
                    }
                    
                    if (DrawButton(font, Rectangle{ 60, 320, 380, 45 }, "Искать приключения", btnNormal, btnHover, btnClick, textWhite)) {
                        ExploreEvent ev = exploreMeadowSearch(player);
                        AddLog(ev.message, eventLog);
                        if (ev.isSlime) {
                            state = STATE_MEADOW_SLIME;
                        }
                    }
                    
                    if (DrawButton(font, Rectangle{ 60, 390, 380, 45 }, "Вернуться в лагерь", Color{ 75, 85, 99, 255 }, Color{ 107, 114, 128, 255 }, Color{ 55, 65, 81, 255 }, textWhite)) {
                        AddLog("Вы вернулись в безопасный лагерь.", eventLog);
                        state = STATE_CAMP;
                    }
                } 
                else if (state == STATE_MEADOW_SLIME) {
                    // Кнопки при встрече со Слизью (интерактивный бой/дружба)
                    DrawTextEx(font, "Перед вами Слизь! Выберите действие:", Vector2{ 50, 190 }, 16, 1.0f, textWhite);
                    
                    // Рисуем симпатичную визуальную Слизь (зеленый кружок с глазками) в углу панели
                    Vector2 slimeCenter = { 250, 275 };
                    DrawCircleV(slimeCenter, 45, Color{ 34, 197, 94, 200 });
                    DrawCircleV(slimeCenter, 40, Color{ 34, 197, 94, 255 });
                    // Глаза слизи
                    DrawCircle(slimeCenter.x - 12, slimeCenter.y - 8, 4, textWhite);
                    DrawCircle(slimeCenter.x + 12, slimeCenter.y - 8, 4, textWhite);
                    DrawCircle(slimeCenter.x - 12, slimeCenter.y - 8, 2, bgDark);
                    DrawCircle(slimeCenter.x + 12, slimeCenter.y - 8, 2, bgDark);
                    
                    if (DrawButton(font, Rectangle{ 60, 350, 380, 45 }, "Попытаться подружиться (риск)", Color{ 16, 185, 129, 255 }, Color{ 52, 211, 153, 255 }, Color{ 4, 120, 87, 255 }, textWhite)) {
                        std::string res = handleSlimeBefriend(player);
                        AddLog(res, eventLog);
                        if (!player.isAlive) {
                            state = STATE_GAME_OVER;
                        } else {
                            state = STATE_MEADOW;
                        }
                    }
                    
                    if (DrawButton(font, Rectangle{ 60, 410, 380, 45 }, "Атаковать слизь", Color{ 239, 68, 68, 255 }, Color{ 248, 113, 113, 255 }, Color{ 185, 28, 28, 255 }, textWhite)) {
                        std::string res = handleSlimeAttack(player);
                        AddLog(res, eventLog);
                        state = STATE_MEADOW;
                    }
                    
                    if (DrawButton(font, Rectangle{ 60, 470, 380, 45 }, "Сбежать", Color{ 107, 114, 128, 255 }, Color{ 156, 163, 175, 255 }, Color{ 75, 85, 99, 255 }, textWhite)) {
                        std::string res = handleSlimeRun(player);
                        AddLog(res, eventLog);
                        state = STATE_MEADOW;
                    }
                }
                
                // 2. Правая панель (Журнал событий)
                Rectangle logPanel = { 490, 120, 430, 500 };
                DrawRectangleRounded(logPanel, 0.05f, 4, bgPanel);
                DrawRectangleRoundedLines(logPanel, 0.05f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                DrawTextEx(font, "ЖУРНАЛ СОБЫТИЙ", Vector2{ 510, 140 }, 20, 1.0f, textGray);
                DrawLine(510, 170, 900, 170, Color{ 48, 54, 68, 255 });
                
                // Вывод последних 13 строк журнала с градиентным затуханием (прозрачностью)
                int startIdx = std::max(0, (int)eventLog.size() - 13);
                int endIdx = eventLog.size();
                int drawCount = endIdx - startIdx;
                int startY = 190;
                
                for (int i = 0; i < drawCount; ++i) {
                    std::string msg = eventLog[startIdx + i];
                    float opacity = (float)(i + 1) / (float)drawCount;
                    Color color = Color{ 243, 244, 246, (unsigned char)(255 * opacity) };
                    
                    // Специфический цвет для важных типов сообщений
                    if (msg.find("[СБОР]") != std::string::npos || msg.find("[УДАЧА]") != std::string::npos || msg.find("нашли") != std::string::npos) {
                        color = Color{ 34, 197, 94, (unsigned char)(255 * opacity) }; // Зеленый для успеха/сбора
                    } else if (msg.find("[СМЕРТЬ]") != std::string::npos || msg.find("погибли") != std::string::npos || msg.find("[БОЙ]") != std::string::npos) {
                        color = Color{ 239, 68, 68, (unsigned char)(255 * opacity) }; // Красный для боя/смерти
                    } else if (msg.find("[СЛИЗЬ]") != std::string::npos || msg.find("ВСТРЕЧА") != std::string::npos) {
                        color = Color{ 245, 158, 11, (unsigned char)(255 * opacity) }; // Золотой для особых встреч
                    }
                    
                    DrawWrappedText(font, msg.c_str(), 510, startY, 390, 14, color);
                    
                    // Считаем размер блока текста с автопереносом, чтобы правильно позиционировать следующую строку
                    // Будем просто сдвигаться на 30 пикселей для простоты
                    startY += 32;
                }
                break;
            }
            case STATE_GAME_OVER: {
                // Экран поражения героя
                DrawTextEx(font, "ГЕРОЙ ПАЛ В БОЮ", Vector2{ 350, 180 }, 32, 1.0f, Color{ 239, 68, 68, 255 });
                DrawTextEx(font, "Ваше приключение закончилось гибелью героя...", Vector2{ 290, 240 }, 16, 1.0f, textGray);
                
                // Характеристики героя на момент смерти
                Rectangle statsRect = { 325, 290, 300, 130 };
                DrawRectangleRounded(statsRect, 0.1f, 4, bgPanel);
                DrawRectangleRoundedLines(statsRect, 0.1f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                std::string fn = "Имя: " + player.name;
                DrawTextEx(font, fn.c_str(), Vector2{ statsRect.x + 20, statsRect.y + 20 }, 16, 1.0f, textWhite);
                
                std::string fc = "Класс: " + player.className;
                DrawTextEx(font, fc.c_str(), Vector2{ statsRect.x + 20, statsRect.y + 50 }, 16, 1.0f, textWhite);
                
                std::stringstream ssG;
                ssG << "Собрано золота: " << player.gold;
                DrawTextEx(font, ssG.str().c_str(), Vector2{ statsRect.x + 20, statsRect.y + 80 }, 16, 1.0f, Color{ 245, 158, 11, 255 });
                
                // Кнопка возрождения
                if (DrawButton(font, Rectangle{ 375, 460, 200, 45 }, "Начать сначала", btnNormal, btnHover, btnClick, textWhite)) {
                    playerNameInput = "";
                    eventLog.clear();
                    AddLog("Добро пожаловать в Новое Приключение!", eventLog);
                    state = STATE_WELCOME;
                }
                break;
            }
        }

        EndDrawing();
    }

    // 5. Очистка ресурсов
    UnloadFont(font);
    CloseWindow();
    return 0;
}
