#pragma once
#include "player.h"
#include <string>

// ИНТЕРФЕЙС БИОМОВ (Biome Interface)
// Неблокирующие функции для взаимодействия с игровым миром

// Результат поиска приключений
struct ExploreEvent {
    bool isSlime;           // Привело ли событие к встрече со слизью
    std::string message;    // Описание события
};

// Инициализация генератора случайных чисел
void initBiomeRandom();

// Действие: Осмотреться
std::string exploreMeadowLookAround();

// Действие: Сбор трав
std::string exploreMeadowGatherHerbs(Player& player);

// Действие: Искать приключения
ExploreEvent exploreMeadowSearch(Player& player);

// Выбор встречи: Подружиться со слизью
std::string handleSlimeBefriend(Player& player);

// Выбор встречи: Атаковать слизь
std::string handleSlimeAttack(Player& player);

// Выбор встречи: Сбежать от слизи
std::string handleSlimeRun(Player& player);
