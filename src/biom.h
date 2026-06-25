#pragma once
#include "player.h"
#include <string>

// Результат поиска приключений
struct ExploreEvent {
    bool isSlime;           // встретили слизь или нет
    std::string message;    // описание что произошло
};

// Инициализация рандома
void initBiomeRandom();

// Осмотр местности
std::string exploreMeadowLookAround();

// Сбор трав и ресурсов
std::string exploreMeadowGatherHerbs(Player& player);

// Поиск приключений на поляне
ExploreEvent exploreMeadowSearch(Player& player);

// Попытка подружиться со слизью
std::string handleSlimeBefriend(Player& player);

// Атака по слизи
std::string handleSlimeAttack(Player& player);

// Побег от слизи
std::string handleSlimeRun(Player& player);
