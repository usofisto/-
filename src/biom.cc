#include "biom.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>

// Инициализация генератора случайных чисел один раз
void initBiomeRandom() {
    static bool randInitialized = false;
    if (!randInitialized) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        randInitialized = true;
    }
}

/**
 * @brief Описание действия "Осмотреться"
 * @return Текстовое описание местности
 */
std::string exploreMeadowLookAround() {
    return "[ОСМОТР] Вы видите бескрайнее зеленое поле. На горизонте возвышаются темные горы. Под ногами ползают безобидные жучки. Это место кажется мирным.";
}

/**
 * @brief Действие сбора ресурсов на поляне (целебные травы / золотоцвет / ничего)
 * @param player Ссылка на структуру игрока для изменения здоровья/золота
 * @return Текстовый результат сбора
 */
std::string exploreMeadowGatherHerbs(Player& player) {
    initBiomeRandom();
    int rnd = std::rand() % 100;

    std::stringstream ss;
    if (rnd < 40) {
        // 40% шанс найти лечебную траву (восстановление 15-25 HP)
        int healAmount = 15 + (std::rand() % 11);
        int oldHealth = player.health;
        player.health = std::min(player.maxHealth, player.health + healAmount);
        int actualHealed = player.health - oldHealth;

        ss << "[СБОР] Вы нашли подорожник-траву! Здоровье восстановлено на " 
           << actualHealed << " HP. (Текущее HP: " << player.health << "/" << player.maxHealth << ")";
    } 
    else if (rnd < 75) {
        // 35% шанс найти золотой цветок (10-25 золота)
        int goldAmount = 10 + (std::rand() % 16);
        player.gold += goldAmount;

        ss << "[СБОР] О! Вы нашли редкий Золотоцвет! Вы бережно сорвали его (+" << goldAmount << " золота).";
    } 
    else {
        // 25% шанс ничего не найти
        ss << "[СБОР] К сожалению, вы не нашли ничего полезного, кроме обычной крапивы.";
    }

    return ss.str();
}

/**
 * @brief Поиск приключений (случайное существо или событие)
 * @param player Ссылка на игрока
 * @return Структура с флагом встречи со слизью и текстовым описанием
 */
ExploreEvent exploreMeadowSearch(Player& player) {
    initBiomeRandom();
    int encounter = std::rand() % 100;

    ExploreEvent event;
    if (encounter < 35) {
        // 35% шанс пустой поляны
        event.isSlime = false;
        event.message = "[ПОИСК] Вы никого не встретили. Только испуганный заяц пронесся мимо.";
    } 
    else {
        // 65% шанс встретить Зеленую Слизь
        event.isSlime = true;
        event.message = "[ПОИСК] !!! ВСТРЕЧА !!! Перед вами прыгает полупрозрачная Зеленая Слизь! Она с любопытством перекатывается сбоку на бок.";
    }
    return event;
}

/**
 * @brief Попытка приручить слизь (рискованное действие)
 * @param player Ссылка на игрока для изменения характеристик
 * @return Текстовое описание исхода
 */
std::string handleSlimeBefriend(Player& player) {
    initBiomeRandom();
    std::stringstream ss;

    int fatalityChance = std::rand() % 100;
    if (fatalityChance < 20) {
        // 20% шанс мгновенной гибели
        player.health = 0;
        player.isAlive = false;

        ss << "[СЛИЗЬ] О нет! Слизь оказалась слишком жидкой и липкой! Она полностью залепила вам лицо, проникнув в рот и нос, лишив вас кислорода. Вы задохнулись и погибли.";
    } 
    else {
        // 80% шанс получить постоянный бафф характеристик
        int dmgGain = 3 + (std::rand() % 5);   // +3..+7 к урону
        int hpGain = 10 + (std::rand() % 11);  // +10..+20 к макс. здоровью
        
        player.damage += dmgGain;
        player.maxHealth += hpGain;
        player.health = std::min(player.maxHealth, player.health + hpGain);

        ss << "[СЛИЗЬ] Невероятно! Слизь забавно устроилась на вашей макушке. Она выделяет приятное тепло! "
           << "Макс. урон: +" << dmgGain << " (Итого: " << player.damage << "), "
           << "Макс. HP: +" << hpGain << " (Итого: " << player.maxHealth << "). Слизь растаяла, оставив шлейф энергии.";
    }

    return ss.str();
}

/**
 * @brief Атака и уничтожение слизи
 * @param player Ссылка на игрока для начисления золота
 * @return Текстовый результат боя
 */
std::string handleSlimeAttack(Player& player) {
    initBiomeRandom();
    std::stringstream ss;

    int goldLoot = 15 + (std::rand() % 16); // 15-30 золота
    player.gold += goldLoot;

    ss << "[БОЙ] Вы обнажаете оружие и наносите резкий удар! Вы разрываете слизь на мелкие кусочки, нанеся " 
       << player.damage << " урона. В останках слизи вы находите монеты (+" << goldLoot << " золота).";

    return ss.str();
}

/**
 * @brief Побег от слизи
 * @param player Ссылка на игрока (не меняется)
 * @return Текстовое описание побега
 */
std::string handleSlimeRun(Player& player) {
    return "[СЛИЗЬ] Вы медленно пятились назад и успешно скрылись в высокой траве. Слизь осталась позади.";
}
