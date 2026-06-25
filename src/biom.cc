#include "biom.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>

// Сид генератора, вызываем один раз при старте
void initBiomeRandom() {
    static bool randInitialized = false;
    if (!randInitialized) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        randInitialized = true;
    }
}

// Осмотр поляны, просто описание что вижу
std::string exploreMeadowLookAround() {
    return "[ОСМОТР] Огляделся: бескрайнее зелёное поле, на горизонте тёмные горы. Под ногами ползают жучки. Тут вроде спокойно.";
}

// Сбор ресурсов: трава / золотоцвет / пусто
std::string exploreMeadowGatherHerbs(Player& player) {
    initBiomeRandom();
    int rnd = std::rand() % 100;

    std::stringstream ss;
    if (rnd < 40) {
        // 40% - лечебная трава
        int healAmount = 15 + (std::rand() % 11);
        int oldHealth = player.health;
        player.health = std::min(player.maxHealth, player.health + healAmount);
        int actualHealed = player.health - oldHealth;

        ss << "[СБОР] Нашёл подорожник, приложил к ранам. Подлечился на " 
           << actualHealed << " HP. (Сейчас HP: " << player.health << "/" << player.maxHealth << ")";
    } 
    else if (rnd < 75) {
        // 35% - золотоцвет
        int goldAmount = 10 + (std::rand() % 16);
        player.gold += goldAmount;

        ss << "[СБОР] О, редкий Золотоцвет! Аккуратно сорвал (+" << goldAmount << " золота).";
    } 
    else {
        // 25% - ничего
        ss << "[СБОР] Обшарил всё вокруг, но ничего кроме крапивы не нашёл.";
    }

    return ss.str();
}

// Поиск приключений: кого-то встретил или нет
ExploreEvent exploreMeadowSearch(Player& player) {
    initBiomeRandom();
    int encounter = std::rand() % 100;

    ExploreEvent event;
    if (encounter < 35) {
        // 35% - пусто
        event.isSlime = false;
        event.message = "[ПОИСК] Никого. Только заяц шуганулся из-под куста и умчался.";
    } 
    else {
        // 65% - слизь
        event.isSlime = true;
        event.message = "[ПОИСК] !!! Из кустов выпрыгнула Зелёная Слизь! Прыгает, пялится на меня.";
    }
    return event;
}

// Попытка подружиться со слизью (рискованно)
std::string handleSlimeBefriend(Player& player) {
    initBiomeRandom();
    std::stringstream ss;

    int fatalityChance = std::rand() % 100;
    if (fatalityChance < 20) {
        // 20% - мгновенная гибель
        player.health = 0;
        player.isAlive = false;

        ss << "[СЛИЗЬ] Чёрт, слизь залепила мне лицо! Не могу дышать... всё.";
    } 
    else {
        // 80% - бафф характеристик
        int dmgGain = 3 + (std::rand() % 5);
        int hpGain = 10 + (std::rand() % 11);
        
        player.damage += dmgGain;
        player.maxHealth += hpGain;
        player.health = std::min(player.maxHealth, player.health + hpGain);

        ss << "[СЛИЗЬ] Ого! Слизь залезла на голову и... тепло! Чувствую прилив сил. "
           << "Урон: +" << dmgGain << " (итого: " << player.damage << "), "
           << "Макс. HP: +" << hpGain << " (итого: " << player.maxHealth << "). Слизь растаяла, оставив энергию.";
    }

    return ss.str();
}

// Бой со слизью: бьём и собираем лут
std::string handleSlimeAttack(Player& player) {
    initBiomeRandom();
    std::stringstream ss;

    int goldLoot = 15 + (std::rand() % 16);
    player.gold += goldLoot;

    ss << "[БОЙ] Рубанул слизь! Разлетелась на куски, нанёс " 
       << player.damage << " урона. В останках нашёл монеты (+" << goldLoot << " золота).";

    return ss.str();
}

// Побег от слизи
std::string handleSlimeRun(Player& player) {
    return "[СЛИЗЬ] Тихо попятился назад и спрятался в высокой траве. Слизь осталась позади, фух.";
}
