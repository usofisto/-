#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include "biom.h" // Подключаем интерфейс биомов и структуру Player

int main() {
    setlocale(LC_ALL, "Russian");
    std::cout << "Привет, мир!" << std::endl;
    
    std::cout << "Привет, для начала выбери никнейм персонажу: ";
    std::string player_name;
    std::cin >> player_name;

    std::cout << "Отлично, " << player_name << ", теперь давай определимся с классом персонажа.\n\n";

    std::cout << "1. Воин (Здоровье: 120, Урон: 10)\n";
    std::cout << "2. Маг (Здоровье: 80, Урон: 20)\n";
    std::cout << "3. Лучник (Здоровье: 100, Урон: 15)\n";
    std::cout << "Выбери класс (1-3): ";

    int player_class {0};
    std::cin >> player_class;
    
    Player player;
    player.name = player_name;
    player.gold = 100; // Стартовое золото
    player.isAlive = true;

    switch (player_class) {
        case 1:
            player.className = "Воин";
            player.maxHealth = 120;
            player.health = 120;
            player.damage = 10;
            std::cout << "\nТы выбрал Воина. Сила и выносливость - твое всё!\n";
            break;
        case 2:
            player.className = "Маг";
            player.maxHealth = 80;
            player.health = 80;
            player.damage = 20;
            std::cout << "\nТы выбрал Мага. Тайная магия сокрушит твоих врагов!\n";
            break;
        case 3:
            player.className = "Лучник";
            player.maxHealth = 100;
            player.health = 100;
            player.damage = 15;
            std::cout << "\nТы выбрал Лучника. Меткость и скорость - твои козыри!\n";
            break;
        default:
            player.className = "Авантюрист";
            player.maxHealth = 100;
            player.health = 100;
            player.damage = 5;
            std::cout << "\nВыбор не распознан. Создан класс по умолчанию: " << player.className << "\n";
            break;
    }

    std::cout << "\n Персонаж успешно создан! \n";
    std::cout << "Имя: " << player.name << "\n";
    std::cout << "Класс: " << player.className << "\n";
    std::cout << "Здоровье: " << player.health << "/" << player.maxHealth << "\n";
    std::cout << "Урон: " << player.damage << "\n";
    std::cout << "Золото: " << player.gold << "\n";
    std::cout << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n";

    
    // ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ (Main Game Loop)
    // Предоставляет игроку свободу перемещения между локациями
    
    bool gameRunning = true;
    while (gameRunning && player.isAlive) {
        std::cout << "\n[ГЛАВНОЕ МЕНЮ] Что вы хотите сделать?\n";
        std::cout << "1. Отправиться на Зеленую Поляну (Первый биом)\n";
        std::cout << "2. Показать статус персонажа\n";
        std::cout << "3. Завершить приключение и выйти\n";
        std::cout << "Выберите действие (1-3): ";

        int mainChoice = 0;
        std::cin >> mainChoice;

        // Защита от ввода некорректных символов (например, букв)
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
            std::cout << "Пожалуйста, введите число 1, 2 или 3.\n";
            continue;
        }

        switch (mainChoice) {
            case 1:
                // Переход в биом Поляны
                enterMeadowBiome(player);
                break;
            case 2:
                // Показать статус игрока
                std::cout << "\n--------------------------------------------\n";
                std::cout << " ТЕКУЩИЙ СТАТУС ГЕРОЯ:\n";
                std::cout << " Имя: " << player.name << "\n";
                std::cout << " Класс: " << player.className << "\n";
                std::cout << " Здоровье: " << player.health << "/" << player.maxHealth << " HP\n";
                std::cout << " Урон: " << player.damage << "\n";
                std::cout << " Золото: " << player.gold << "\n";
                std::cout << "--------------------------------------------\n";
                break;
            case 3:
                std::cout << "\nСпасибо за игру! До новых встреч!\n";
                gameRunning = false;
                break;
            default:
                std::cout << "Неверный выбор. Пожалуйста, введите число от 1 до 3.\n";
                break;
        }
    }

    if (!player.isAlive) {
        std::cout << "\n[КОНЕЦ] К сожалению, ваш герой пал в бою или из-за несчастного случая.\n";
        std::cout << "Игра окончена. Попробуйте еще раз!\n";
    }

    return 0;
}
