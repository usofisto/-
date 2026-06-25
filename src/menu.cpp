#include "menu.h"
#include "raylib.h"
#include <string>
#include <cmath>

GameMenu::GameMenu() {
    currentState = MENU_MAIN;
    selectedWorldIndex = -1;
    worldNameInput = "Новый мир";
    worldSettings = {"Новый мир", 200, false, 1};
    worldList = {"Лесной берег", "Горный лес", "Пустынные холмы"};
}

bool GameMenu::DrawMenuButton(Font font, Rectangle rect, const char* text, bool isActive) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), rect);

    Color bgColor;
    Color textColor;

    if (isHovered) {
        bgColor = Color{50, 50, 50, 255};
        textColor = Color{200, 200, 200, 255};
    } else if (isActive) {
        bgColor = Color{80, 80, 80, 255};
        textColor = Color{255, 255, 255, 255};
    } else {
        bgColor = Color{60, 60, 60, 255};
        textColor = Color{180, 180, 180, 255};
    }

    DrawRectangleRec(rect, bgColor);
    DrawRectangleLinesEx(rect, 2, Color{100, 100, 100, 255});

    Vector2 textSize = MeasureTextEx(font, text, 20, 1);
    Vector2 textPos = {rect.x + (rect.width - textSize.x) / 2, rect.y + (rect.height - textSize.y) / 2};
    DrawTextEx(font, text, textPos, 20, 1, textColor);

    if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        DrawRectangleRec(rect, Color{100, 100, 100, 100});
        return true;
    }
    return false;
}

bool GameMenu::DrawTextInput(Font font, Rectangle rect, std::string& input, bool isActive) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), rect);
    static bool cursorVisible = true;
    static int cursorTimer = 0;

    if (isActive) {
        cursorTimer++;
        if (cursorTimer > 30) { cursorVisible = !cursorVisible; cursorTimer = 0; }
    }

    Color bgColor = isActive ? Color{40, 40, 40, 255} : Color{30, 30, 30, 255};
    DrawRectangleRec(rect, bgColor);

    Color borderColor = isActive ? Color{100, 150, 200, 255} : Color{80, 80, 80, 255};
    DrawRectangleLinesEx(rect, 2, borderColor);

    Vector2 textSize = MeasureTextEx(font, input.c_str(), 18, 1);
    Vector2 textPos = {rect.x + 10, rect.y + (rect.height - textSize.y) / 2};
    DrawTextEx(font, input.c_str(), textPos, 18, 1, Color{200, 200, 200, 255});

    if (isActive && cursorVisible) {
        float cursorX = rect.x + 10 + textSize.x + 2;
        DrawRectangle((int)cursorX, (int)(rect.y + 10), 2, (int)(rect.height - 20), Color{200, 200, 200, 255});
    }

    if (isActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key != '\n' && key != '\r') input += (char)key;
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !input.empty()) input.pop_back();
    }
    return isHovered;
}

bool GameMenu::DrawSlider(Font font, Rectangle rect, int& value, int minVal, int maxVal, const char* label) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), rect);

    DrawRectangleRec(rect, Color{40, 40, 40, 255});

    float fillRatio = (float)(value - minVal) / (float)(maxVal - minVal);
    Rectangle fillRect = {rect.x + 2, rect.y + 2, (rect.width - 4) * fillRatio, rect.height - 4};
    DrawRectangleRec(fillRect, Color{80, 120, 160, 255});

    float handleX = rect.x + (rect.width - 8) * fillRatio;
    DrawRectangle((int)handleX, (int)(rect.y + 4), 8, (int)(rect.height - 8), Color{180, 180, 180, 255});

    DrawTextEx(font, label, {rect.x + rect.width + 10, rect.y + 5}, 16, 1, Color{180, 180, 180, 255});
    DrawTextEx(font, TextFormat("%d", value), {rect.x + rect.width + 100, rect.y + 5}, 16, 1, Color{255, 255, 255, 255});

    if (isHovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float mouseRatio = (GetMousePosition().x - rect.x) / rect.width;
        value = minVal + (int)(mouseRatio * (maxVal - minVal));
        if (value < minVal) value = minVal;
        if (value > maxVal) value = maxVal;
        return true;
    }
    return false;
}

void GameMenu::DrawMainMenu(Font font, int windowWidth, int windowHeight, int& framesCounter) {
    ClearBackground(Color{20, 20, 20, 255});

    for (int i = 0; i < 50; i++) {
        float x = fmodf((float)(i * 37 + framesCounter / 30), (float)windowWidth);
        float y = fmodf((float)(i * 23), (float)windowHeight);
        float size = 1.0f + (float)(i % 3);
        float alpha = 0.3f + 0.3f * sinf((float)framesCounter / 30.0f + (float)i);
        unsigned char a = (unsigned char)(alpha * 255);
        DrawCircleV({x, y}, size, Color{255, 255, 255, a});
    }

    const char* title = "LEGENDS OF GREEN MEADOW";
    Vector2 titleSize = MeasureTextEx(font, title, 40, 1);
    Vector2 titlePos = {(windowWidth - titleSize.x) / 2, 80};
    DrawTextEx(font, title, {titlePos.x + 2, titlePos.y + 2}, 40, 1, Color{0, 0, 0, 100});
    DrawTextEx(font, title, titlePos, 40, 1, Color{255, 255, 255, 255});

    const char* subtitle = "Тихий лесной берег";
    Vector2 subtitleSize = MeasureTextEx(font, subtitle, 18, 1);
    DrawTextEx(font, subtitle, {(windowWidth - subtitleSize.x) / 2, 140}, 18, 1, Color{150, 150, 150, 255});

    float buttonWidth = 250, buttonHeight = 40;
    float buttonX = (windowWidth - buttonWidth) / 2;
    float startY = 200, spacing = 55;

    if (DrawMenuButton(font, {buttonX, startY, buttonWidth, buttonHeight}, "ОДИНОЧНАЯ ИГРА", true))
        currentState = MENU_SINGLEPLAYER;
    DrawMenuButton(font, {buttonX, startY + spacing, buttonWidth, buttonHeight}, "СЕТЕВАЯ ИГРА", false);
    DrawMenuButton(font, {buttonX, startY + spacing * 2, buttonWidth, buttonHeight}, "МИНИ-ИГРЫ", false);
    if (DrawMenuButton(font, {buttonX, startY + spacing * 3, buttonWidth, buttonHeight}, "НАСТРОЙКИ", false))
        currentState = MENU_SETTINGS;
    if (DrawMenuButton(font, {buttonX, startY + spacing * 4, buttonWidth, buttonHeight}, "ВЫЙТИ", false))
        CloseWindow();

    DrawTextEx(font, "Версия 0.1.0 | Разработка", {20, (float)(windowHeight - 30)}, 14, 1, Color{100, 100, 100, 255});
}

void GameMenu::DrawWorldCreateMenu(Font font, int windowWidth, int windowHeight) {
    ClearBackground(Color{20, 20, 20, 255});

    const char* title = "СОЗДАНИЕ МИРА";
    Vector2 titleSize = MeasureTextEx(font, title, 32, 1);
    DrawTextEx(font, title, {(windowWidth - titleSize.x) / 2, 50}, 32, 1, Color{255, 255, 255, 255});

    float startY = 120, spacing = 50;
    float leftCol = 100, rightCol = 350;

    DrawTextEx(font, "Название:", {leftCol, startY}, 20, 1, Color{180, 180, 180, 255});
    DrawTextInput(font, {rightCol, startY - 5, 300, 30}, worldNameInput, true);

    startY += spacing;
    DrawTextEx(font, "Размер мира:", {leftCol, startY}, 20, 1, Color{180, 180, 180, 255});
    DrawSlider(font, {rightCol, startY, 200, 25}, worldSettings.worldSize, 100, 500, "");

    startY += spacing;
    DrawTextEx(font, "Сложность:", {leftCol, startY}, 20, 1, Color{180, 180, 180, 255});
    const char* difficulties[] = {"Легко", "Нормально", "Сложно"};
    for (int i = 0; i < 3; i++) {
        Rectangle btnRect = {rightCol + (float)i * 100, startY - 5, 90, 30};
        bool isSelected = (worldSettings.difficulty == i);
        DrawMenuButton(font, btnRect, difficulties[i], isSelected);
        if (isSelected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            worldSettings.difficulty = i;
    }

    startY += spacing;
    DrawTextEx(font, "Чит-коды:", {leftCol, startY}, 20, 1, Color{180, 180, 180, 255});
    Rectangle toggleRect = {rightCol, startY - 5, 100, 30};
    const char* toggleText = worldSettings.cheatsEnabled ? "ВКЛ" : "ВЫКЛ";
    DrawMenuButton(font, toggleRect, toggleText, false);
    if (CheckCollisionPointRec(GetMousePosition(), toggleRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        worldSettings.cheatsEnabled = !worldSettings.cheatsEnabled;

    float buttonWidth = 150, buttonHeight = 35, buttonY = (float)(windowHeight - 80);

    if (DrawMenuButton(font, {(windowWidth - buttonWidth * 2 - 20) / 2.0f, buttonY, buttonWidth, buttonHeight}, "СОЗДАТЬ", true)) {
        worldSettings.worldName = worldNameInput;
        currentState = MENU_PLAYING;
    }
    if (DrawMenuButton(font, {(windowWidth + 20) / 2.0f, buttonY, buttonWidth, buttonHeight}, "НАЗАД", false))
        currentState = MENU_MAIN;
}

void GameMenu::DrawSettingsMenu(Font font, int windowWidth, int windowHeight) {
    ClearBackground(Color{20, 20, 20, 255});

    const char* title = "НАСТРОЙКИ";
    Vector2 titleSize = MeasureTextEx(font, title, 32, 1);
    DrawTextEx(font, title, {(windowWidth - titleSize.x) / 2, 50}, 32, 1, Color{255, 255, 255, 255});

    const char* comingSoon = "Настройки будут доступны в следующем обновлении";
    Vector2 csSize = MeasureTextEx(font, comingSoon, 18, 1);
    DrawTextEx(font, comingSoon, {(windowWidth - csSize.x) / 2, 200}, 18, 1, Color{150, 150, 150, 255});

    if (DrawMenuButton(font, {(windowWidth - 150) / 2.0f, 300, 150, 35}, "НАЗАД", true))
        currentState = MENU_MAIN;
}

bool GameMenu::HandleMainMenuInput(Font font, int windowWidth, int windowHeight) { return false; }
bool GameMenu::HandleWorldCreateInput(Font font, int windowWidth, int windowHeight) { return false; }
bool GameMenu::HandleSettingsInput(Font font, int windowWidth, int windowHeight) { return false; }
