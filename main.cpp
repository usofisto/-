#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

int main() {
    setlocale(LC_ALL, "Russian");
    std::cout << "Привет, мир!" << std::endl;
    
    std::cout << "Привет, для начало выбери никнейм персонажу:";
    std::string player_name;
    std::cin >> player_name;

    std::cout << "Отлично, " << player_name << ", теперь давай определимся с классом персонажа.\n";

    // Воин

    std::cout << "1. Воин\n";
    // выносливость
    // сила
    // броня
    // запас здоровья

    std::cout << "2. Маг\n";
    // выносливость
    // сила
    // броня
    // запас здоровья
    
    std::cout << "3. Лучник\n";
    // выносливость
    // сила
    // броня
    // запас здоровья

    int player_class {0};
    std::cin >> player_class;
    short damage {0};
    
    // Закомментировал незавершенный блок, чтобы код компилировался:
    /*
    if () {
        damage = 10;
    } else if (player_class == 2) {
        damage = 20;
    } else if (player_class == 3) {
        damage = 30;
    }
    */

    switch (player_class) {
        case 1:
            std::cout << "Ты выбрал воина\n";
            damage = 5;
            break;
        case 2:
            std::cout << "Ты выбрал мага\n";
            damage = 10;
            break;
        case 3:
            std::cout << "Ты выбрал лучника\n";
            damage = 15;
            break;
        default:
            std::cout << "Ты выбрал непонятно кого\n";
            break;
    }

    // мне стало лень пока доделывать на этот день все. Завтра допишу.

    return 0;
}
