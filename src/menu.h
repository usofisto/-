#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Состояния меню
enum MenuState {
    MENU_MAIN,           // Главное меню
    MENU_SINGLEPLAYER,   // Одиночная игра
    MENU_MULTIPLAYER,    // Мультиплеер (заглушка)
    MENU_SETTINGS,       // Настройки
    MENU_WORLD_CREATE,   // Создание мира
    MENU_WORLD_LOAD,     // Загрузка мира
    MENU_PLAYING         // Игра
};

// Настройки мира
struct WorldSettings {
    std::string worldName;
    int worldSize;        // Размер мира (100x100, 200x200 и т.д.)
    bool cheatsEnabled;   // Включены ли чит-коды
    int difficulty;       // Сложность (0-легко, 1-нормально, 2-сложно)
};

// Класс меню
class GameMenu {
public:
    GameMenu();
    
    // Отрисовка меню
    void DrawMainMenu(Font font, int windowWidth, int windowHeight, int& framesCounter);
    void DrawWorldCreateMenu(Font font, int windowWidth, int windowHeight);
    void DrawSettingsMenu(Font font, int windowWidth, int windowHeight);
    
    // Обработка ввода
    bool HandleMainMenuInput(Font font, int windowWidth, int windowHeight);
    bool HandleWorldCreateInput(Font font, int windowWidth, int windowHeight);
    bool HandleSettingsInput(Font font, int windowWidth, int windowHeight);
    
    // Получение настроек
    WorldSettings GetWorldSettings() const { return worldSettings; }
    MenuState GetCurrentState() const { return currentState; }
    void SetCurrentState(MenuState state) { currentState = state; }
    
    // Поля ввода
    std::string worldNameInput;
    
private:
    MenuState currentState;
    WorldSettings worldSettings;
    int selectedWorldIndex;
    std::vector<std::string> worldList;
    
    // Кнопки меню
    bool DrawMenuButton(Font font, Rectangle rect, const char* text, bool isActive);
    bool DrawTextInput(Font font, Rectangle rect, std::string& input, bool isActive);
    bool DrawSlider(Font font, Rectangle rect, int& value, int minVal, int maxVal, const char* label);
};
