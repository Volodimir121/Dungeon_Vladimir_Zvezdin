#pragma once
#include "Map.h"
#include "Hero.h"
#include <string>

/*
 * Управление главным игровым циклом (Практика 9, задание 4).
 *
 * Объединяет объекты двух основных классов проекта — Map и Hero.
 * Порядок объявления полей: сначала map, затем hero.
 * По правилам C++ поля конструируются в порядке их объявления в классе
 * (сначала Map, затем Hero), а разрушаются деструкторами в строго обратном
 * порядке (сначала ~Hero(), затем ~Map()).
 */
class Game
{
public:
    Map  map;
    Hero hero;
    int  turn;
    bool isRunning;

    /* Конструктор с процедурной генерацией карты заданного размера */
    Game(int rows, int cols);

    /* Конструктор с загрузкой карты из файла (Практика 10, задание 6) */
    explicit Game(const std::string& filename);

    ~Game();

    Game(const Game&)            = delete;
    Game& operator=(const Game&) = delete;

    /* Главный цикл: чтение команды, вызов методов объектов, вывод */
    void run();

    /* Демонстрация стека, кучи и блока области видимости (Практика 10, задания 3–4) */
    static void runLifetimeDemo();

private:
    void processCommand(const std::string& line);
    void moveHero(int dx, int dy, const std::string& dirName);
    void checkCurrentCell();
    void printHelp() const;
};
