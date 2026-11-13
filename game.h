#pragma once
#include "Creature.h"
#include "Hero.h"
#include "Map.h"
#include "MapRenderer.h"
#include "Monster.h"

#include <memory>
#include <string>
#include <vector>

/*
 * Game — главный игровой цикл.
 *
 * Практика 12, задание 6, и подсказка про виды связей:
 *   Game  ◆— Map       композиция: карта создаётся и умирает вместе с игрой;
 *   Game  ◆— Hero      композиция: герой принадлежит игре;
 *   Game  ◆— Monster*  композиция: unique_ptr владеют монстрами, при уничтожении
 *                      Game все монстры освобождаются автоматически;
 *   Game  ◆— MapRenderer  композиция (без состояния);
 *   Monster ··> IWorld ассоциация: мир передаётся в takeTurn() на время хода,
 *                      монстр им не владеет;
 *   MapRenderer ··> Map, Creature  зависимость: только читает.
 *   «Клетка содержит предмет» — совсем другая связь: предмет это символ, значение
 *   внутри клетки; клетка без него существует (FLOOR), а предмет вне клетки — нет.
 *
 * Порядок объявления полей: map, hero, monsters, renderer.
 * Конструируются в этом порядке, разрушаются в обратном: сначала монстры,
 * затем ~Hero(), затем ~Map(). Ручное освобождение не нужно (RAII).
 *
 * ИНВАРИАНТ КЛАССА (практика 11, критерий 1):
 *   — turn >= 1;
 *   — в monsters нет нулевых указателей и нет мёртвых монстров
 *     (мёртвые удаляются сразу после гибели);
 *   — герой и монстры стоят на проходимых клетках карты и не делят одну клетку;
 *   — после isRunning == false ни один ход больше не выполняется.
 */
class Game : private IWorld
{
public:
    /* Конструктор с процедурной генерацией карты заданного размера */
    Game(int rows, int cols);

    /* Конструктор с загрузкой карты из файла (практика 10, задание 6) */
    explicit Game(const std::string& filename);

    ~Game() override;

    Game(const Game&)            = delete;
    Game& operator=(const Game&) = delete;

    /* Главный цикл: чтение команды, ход игрока, ход всех монстров, отрисовка */
    void run();

    /* Демонстрация стека, кучи и блока области видимости (практика 10, задания 3–4) */
    static void runLifetimeDemo();

    /* Демонстрация виртуального и невиртуального деструктора (практика 12, задание 7) */
    static void runVirtualDestructorDemo();

private:
    /* Реализация IWorld: то, что монстры могут узнать о мире */
    const Map& getMap() const override;
    Creature&  getPlayer() override;
    bool       isCellOccupied(int x, int y) const override;
    void       report(const std::string& message) override;

    /* Возвращает true, если действие игрока заняло ход */
    bool processCommand(const std::string& line);
    bool moveHero(int dx, int dy, const std::string& dirName);
    void checkCurrentCell();

    /* Ход монстров: единообразный обход контейнера через виртуальный takeTurn() */
    void monstersTakeTurns();

    Monster* monsterAt(int x, int y) const;
    void     removeDeadMonsters();
    void     addMonster(int kind, int x, int y);
    void     spawnMonstersRandomly();
    void     spawnMonstersFromMap();

    void draw() const;
    void printHelp() const;

    Map                                  map;
    Hero                                 hero;
    std::vector<std::unique_ptr<Monster>> monsters;
    MapRenderer                          renderer;
    int                                  turn;
    bool                                 isRunning;
};
