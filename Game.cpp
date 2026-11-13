#include "Game.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

/* -------------------------------------------------------------------------- */
/*  Конструкторы и деструктор                                                 */
/* -------------------------------------------------------------------------- */

Game::Game(int rows, int cols)
    : map(rows, cols, Map::Procedural{}),                               /* 1. Map (куча) */
      hero(map.getStartHeroX(), map.getStartHeroY(), 100, 0),           /* 2. Hero */
      monsters(),                                                       /* 3. контейнер монстров */
      renderer(),
      turn(1),
      isRunning(true)
{
    this->spawnMonstersRandomly();
}

Game::Game(const std::string& filename)
    : map(filename),     /* при ошибке бросит исключение, ~Map() освободит память, Game не создаётся */
      hero(map.getStartHeroX(), map.getStartHeroY(), 100, 0),
      monsters(),
      renderer(),
      turn(1),
      isRunning(true)
{
    this->spawnMonstersFromMap();
    std::cout << "Level loaded from '" << filename << "'.\n";
}

Game::~Game()
{
    /*
     * Порядок вызовов: сначала тело ~Game(), затем АВТОМАТИЧЕСКИ в порядке, обратном
     * объявлению полей: renderer, monsters (каждый unique_ptr удаляет монстра через
     * указатель на Monster; благодаря виртуальному деструктору вызывается деструктор
     * реального наследника), затем ~Hero(), затем ~Map().
     */
    std::cout << "[Game] Деструктор: игра завершена, далее автоматически уничтожатся монстры, Hero и Map.\n";
}

/* -------------------------------------------------------------------------- */
/*  IWorld                                                                    */
/* -------------------------------------------------------------------------- */

const Map& Game::getMap() const
{
    return this->map;
}

Creature& Game::getPlayer()
{
    return this->hero;
}

bool Game::isCellOccupied(int x, int y) const
{
    if (this->hero.isAlive() && this->hero.getX() == x && this->hero.getY() == y)
    {
        return true;
    }
    return this->monsterAt(x, y) != nullptr;
}

void Game::report(const std::string& message)
{
    std::cout << message << "\n";
}

/* -------------------------------------------------------------------------- */
/*  Монстры                                                                   */
/* -------------------------------------------------------------------------- */

Monster* Game::monsterAt(int x, int y) const
{
    for (const std::unique_ptr<Monster>& monster : this->monsters)
    {
        if (monster->isAlive() && monster->getX() == x && monster->getY() == y)
        {
            return monster.get();
        }
    }
    return nullptr;
}

void Game::removeDeadMonsters()
{
    this->monsters.erase(
        std::remove_if(this->monsters.begin(), this->monsters.end(),
                       [](const std::unique_ptr<Monster>& m) { return !m->isAlive(); }),
        this->monsters.end());
}

/*
 * Фабрика видов монстров. Это единственное место, где перечислены конкретные классы;
 * игровой цикл (monstersTakeTurns) работает только с базовым Monster.
 */
void Game::addMonster(int kind, int x, int y)
{
    switch (kind % 3)
    {
        case 0:  this->monsters.push_back(std::make_unique<Zombie>(x, y)); break;
        case 1:  this->monsters.push_back(std::make_unique<Bat>(x, y));    break;
        default: this->monsters.push_back(std::make_unique<Guard>(x, y));  break;
    }
}

void Game::spawnMonstersRandomly()
{
    int wanted = std::max(1, (this->map.getRows() * this->map.getCols()) / 60);
    wanted = std::min(wanted, 8);

    int placed   = 0;
    int attempts = 0;
    while (placed < wanted && attempts < wanted * 200)
    {
        ++attempts;
        int x = 1 + std::rand() % (this->map.getCols() - 2);
        int y = 1 + std::rand() % (this->map.getRows() - 2);

        if (this->map.getCell(x, y) != Map::FLOOR)
        {
            continue;
        }
        if (std::abs(x - this->hero.getX()) + std::abs(y - this->hero.getY()) < 4)
        {
            continue;
        }
        if (this->monsterAt(x, y) != nullptr)
        {
            continue;
        }

        this->addMonster(placed, x, y);
        ++placed;
    }
}

void Game::spawnMonstersFromMap()
{
    int index = 0;
    for (const std::pair<int, int>& spawn : this->map.getMonsterSpawns())
    {
        if (this->isCellOccupied(spawn.first, spawn.second))
        {
            continue;
        }
        this->addMonster(index, spawn.first, spawn.second);
        ++index;
    }
}

void Game::monstersTakeTurns()
{
    /*
     * Практика 12, критерий 1: никакой проверки вида монстра.
     * Каждый takeTurn() вызывается виртуально, поведение определяет реальный класс.
     */
    for (const std::unique_ptr<Monster>& monster : this->monsters)
    {
        monster->takeTurn(*this);

        if (!this->hero.isAlive())
        {
            std::cout << "You have died! Game over.\n";
            this->isRunning = false;
            return;
        }
    }
}

/* -------------------------------------------------------------------------- */
/*  Ход игрока                                                                */
/* -------------------------------------------------------------------------- */

void Game::checkCurrentCell()
{
    char cell = this->map.getCell(this->hero.getX(), this->hero.getY());
    if (cell == Map::GOLD)
    {
        this->hero.addGold(1);
        this->map.setCell(this->hero.getX(), this->hero.getY(), Map::FLOOR);
        std::cout << "You found gold! Total gold: " << this->hero.getGold() << "\n";
    }
    else if (cell == Map::EXIT)
    {
        std::cout << "Congratulations! You reached the exit!\n";
        std::cout << "Total Gold: " << this->hero.getGold() << " | Turns: " << this->turn << "\n";
        this->isRunning = false;
    }
}

bool Game::moveHero(int dx, int dy, const std::string& dirName)
{
    int targetX = this->hero.getX() + dx;
    int targetY = this->hero.getY() + dy;

    if (!this->map.inBounds(targetX, targetY))
    {
        std::cout << "You can't go " << dirName << " - boundary blocks the way.\n";
        return false;
    }

    /* Ход в клетку с монстром — это атака */
    Monster* target = this->monsterAt(targetX, targetY);
    if (target != nullptr)
    {
        const std::string name = target->getName();
        target->takeDamage(Hero::ATTACK_DAMAGE);
        if (target->isAlive())
        {
            std::cout << "You hit the " << name << " for " << Hero::ATTACK_DAMAGE
                      << " damage. It has " << target->getHealth() << " HP left.\n";
        }
        else
        {
            std::cout << "You killed the " << name << "!\n";
            this->removeDeadMonsters();
        }
        return true;
    }

    /* Герой спрашивает у карты: hero.move -> map.canEnter */
    if (!this->hero.move(dx, dy, this->map))
    {
        std::cout << "You can't go " << dirName << " - a wall blocks the way.\n";
        return false;
    }

    std::cout << "You went " << dirName << ".\n";
    this->checkCurrentCell();
    return true;
}

/* -------------------------------------------------------------------------- */
/*  Вывод                                                                     */
/* -------------------------------------------------------------------------- */

void Game::draw() const
{
    std::vector<const Creature*> creatures;
    for (const std::unique_ptr<Monster>& monster : this->monsters)
    {
        creatures.push_back(monster.get());
    }
    creatures.push_back(&this->hero); /* герой рисуется последним, то есть поверх остальных */

    this->renderer.draw(this->map, creatures);
}

void Game::printHelp() const
{
    std::cout << "Available commands (name, short form):\n"
              << "  north     (n)  - go north (moving into a monster attacks it)\n"
              << "  south     (s)  - go south\n"
              << "  east      (e)  - go east\n"
              << "  west      (w)  - go west\n"
              << "  go <dir>       - move in direction (e.g. go north)\n"
              << "  look      (l)  - redraw the dungeon map\n"
              << "  status    (st) - show hero and monster status\n"
              << "  hit <N>        - take N damage (test takeDamage)\n"
              << "  test           - demonstrate stack, heap, and scope block (Practice 10)\n"
              << "  dtor           - demonstrate virtual destructor (Practice 12)\n"
              << "  help      (h)  - show this help\n"
              << "  quit      (q)  - exit the game\n";
}

/* -------------------------------------------------------------------------- */
/*  Демонстрации                                                              */
/* -------------------------------------------------------------------------- */

namespace
{
/*
 * Практика 11: Map больше ничего не выводит, поэтому момент уничтожения карты
 * показывает этот вспомогательный объект. Он объявляется сразу после карты,
 * значит, уничтожается непосредственно перед ней (обратный порядок).
 */
class Tracer
{
public:
    explicit Tracer(std::string name) : name(std::move(name))
    {
        std::cout << "[Tracer] Создана карта: " << this->name << "\n";
    }
    ~Tracer()
    {
        std::cout << "[Tracer] Уничтожается карта: " << this->name << "\n";
    }

private:
    std::string name;
};

/* --- Классы для практики 12, задание 7 --- */

class NonVirtualBase
{
public:
    NonVirtualBase()  { std::cout << "  NonVirtualBase: конструктор\n"; }
    ~NonVirtualBase() { std::cout << "  NonVirtualBase: деструктор\n"; } /* НЕ virtual */
    virtual void speak() const {}
};

class NonVirtualDerived : public NonVirtualBase
{
public:
    NonVirtualDerived() : buffer(new int[100]) { std::cout << "  NonVirtualDerived: конструктор (выделено 400 байт)\n"; }
    ~NonVirtualDerived()
    {
        delete[] this->buffer;
        std::cout << "  NonVirtualDerived: деструктор (буфер освобождён)\n";
    }

private:
    int* buffer;
};

class VirtualBase
{
public:
    VirtualBase()          { std::cout << "  VirtualBase: конструктор\n"; }
    virtual ~VirtualBase() { std::cout << "  VirtualBase: деструктор\n"; } /* virtual */
    virtual void speak() const {}
};

class VirtualDerived : public VirtualBase
{
public:
    VirtualDerived() : buffer(new int[100]) { std::cout << "  VirtualDerived: конструктор (выделено 400 байт)\n"; }
    ~VirtualDerived() override
    {
        delete[] this->buffer;
        std::cout << "  VirtualDerived: деструктор (буфер освобождён)\n";
    }

private:
    int* buffer;
};
}

/*
 * Практика 10, задания 3 и 4: объекты на стеке, во вложенном блоке и в куче.
 *
 * ПОРЯДОК ВЫЗОВОВ КОНСТРУКТОРОВ И ДЕСТРУКТОРОВ:
 * 1. Стековые объекты уничтожаются при выходе из функции в порядке, обратном созданию:
 *    сначала ~Hero (виден в журнале), затем Tracer, затем ~Map.
 * 2. Объект во вложенном блоке уничтожается сразу на '}'.
 * 3. Объект в куче удаляется только явным delete.
 */
void Game::runLifetimeDemo()
{
    std::cout << "--- 1. Stack objects ---\n";
    Map    stackMap(4, 8);
    Tracer stackTracer("stackMap 4x8");
    Hero   stackHero(2, 2, 85, 10);

    std::cout << "\n--- 2. Object inside nested block { ... } ---\n";
    {
        Map    blockMap(3, 5);
        Tracer blockTracer("blockMap 3x5");
    }
    std::cout << "Block left: blockMap is already destroyed.\n";

    std::cout << "\n--- 3. Object on the heap (new / delete) ---\n";
    Map* heapMap = new Map(5, 6);
    std::cout << "heapMap " << heapMap->getRows() << "x" << heapMap->getCols()
              << " created with new; it is still alive here.\n";
    delete heapMap;
    heapMap = nullptr;
    std::cout << "heapMap deleted explicitly.\n";

    std::cout << "\n--- 4. Leaving function (stack objects destroyed in reverse order) ---\n";
}

/*
 * Практика 12, задание 7: удаление наследника через указатель на базовый класс.
 *
 * РЕЗУЛЬТАТ:
 * 1. Если деструктор базового класса НЕ виртуальный, то `delete basePtr` для объекта
 *    наследника — неопределённое поведение (UB) по стандарту C++. На практике
 *    (GCC, Clang, MSVC) вызывается только ~NonVirtualBase(), а ~NonVirtualDerived()
 *    пропускается: буфер в 400 байт, которым владеет наследник, никогда не
 *    освобождается (утечка), а все прочие действия деструктора наследника (закрыть
 *    файл, отписаться и т.п.) теряются. Причина: без virtual вызов деструктора
 *    разрешается статически по типу указателя (NonVirtualBase*), а не по реальному
 *    типу объекта, и таблица виртуальных функций тут не участвует.
 * 2. Если деструктор базового класса виртуальный, то вызов разрешается динамически:
 *    сначала ~VirtualDerived(), затем автоматически ~VirtualBase(). Утечки нет.
 *    Поэтому у Creature деструктор виртуальный: monsters хранит unique_ptr<Monster>,
 *    и без этого Zombie, Bat и Guard уничтожались бы как «голые» Monster.
 * 3. Тот же эффект в настоящей иерархии проекта: delete через Creature* вызывает ~Hero().
 */
void Game::runVirtualDestructorDemo()
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
    std::cout << "--- 1. Non-virtual destructor in base (undefined behavior!) ---\n";
    {
        NonVirtualBase* pointer = new NonVirtualDerived();
        delete pointer; /* UB: ~NonVirtualDerived() обычно не вызывается, буфер течёт */
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    std::cout << "(Look: NonVirtualDerived destructor did NOT run, its buffer leaked.)\n";

    std::cout << "\n--- 2. Virtual destructor in base ---\n";
    {
        VirtualBase* pointer = new VirtualDerived();
        delete pointer;
    }
    std::cout << "(Both destructors ran, in the order derived -> base.)\n";

    std::cout << "\n--- 3. Real hierarchy: delete Hero through Creature* ---\n";
    {
        Creature* creature = new Hero(3, 3, 50, 0);
        delete creature;
    }
    std::cout << "(~Hero ran because ~Creature is virtual.)\n";
}

/* -------------------------------------------------------------------------- */
/*  Команды                                                                   */
/* -------------------------------------------------------------------------- */

bool Game::processCommand(const std::string& rawLine)
{
    std::string lowered = rawLine;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::istringstream iss(lowered);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
    {
        words.push_back(word);
    }

    if (words.empty())
    {
        return false;
    }

    std::string cmd = words[0];
    if (cmd == "go")
    {
        if (words.size() < 2)
        {
            std::cout << "Go where? Example: go north\n";
            return false;
        }
        cmd = words[1];
        if (cmd != "north" && cmd != "n" &&
            cmd != "south" && cmd != "s" &&
            cmd != "east"  && cmd != "e" &&
            cmd != "west"  && cmd != "w")
        {
            std::cout << "You can't go '" << cmd << "'.\n";
            return false;
        }
    }

    if (cmd == "north" || cmd == "n")
    {
        return this->moveHero(0, -1, "north");
    }
    if (cmd == "south" || cmd == "s")
    {
        return this->moveHero(0, 1, "south");
    }
    if (cmd == "east" || cmd == "e")
    {
        return this->moveHero(1, 0, "east");
    }
    if (cmd == "west" || cmd == "w")
    {
        return this->moveHero(-1, 0, "west");
    }

    if (cmd == "look" || cmd == "l")
    {
        std::cout << "You look around the dungeon.\n";
        this->draw();
    }
    else if (cmd == "status" || cmd == "st")
    {
        this->hero.printStatus();
        std::cout << "Monsters alive: " << this->monsters.size()
                  << " (created in total: " << Monster::getTotalCreated() << ")\n";
    }
    else if (cmd == "hit")
    {
        if (words.size() < 2)
        {
            std::cout << "Usage: hit <damage>\n";
            return false;
        }
        try
        {
            int dmg = std::stoi(words[1]);
            if (dmg < 0)
            {
                std::cout << "Damage must be non-negative.\n";
                return false;
            }
            this->hero.takeDamage(dmg);
            std::cout << "You took " << dmg << " damage. Health: "
                      << this->hero.getHealth() << "/" << this->hero.getMaxHealth() << "\n";
            if (!this->hero.isAlive())
            {
                std::cout << "You have died! Game over.\n";
                this->isRunning = false;
            }
        }
        catch (const std::exception&)
        {
            std::cout << "Invalid damage number: '" << words[1] << "'.\n";
        }
    }
    else if (cmd == "test" || cmd == "raii")
    {
        Game::runLifetimeDemo();
    }
    else if (cmd == "dtor")
    {
        Game::runVirtualDestructorDemo();
    }
    else if (cmd == "help" || cmd == "h")
    {
        this->printHelp();
    }
    else if (cmd == "quit" || cmd == "q")
    {
        this->isRunning = false;
    }
    else
    {
        std::cout << "Unknown command: '" << words[0] << "'. Type 'help'.\n";
    }

    return false;
}

void Game::run()
{
    std::cout << "\nWelcome to the Dungeon!\n";
    this->draw();
    std::cout << "\nSymbols: @ - Player, $ - Gold, E - Exit, # - Wall, Z - Zombie, B - Bat, G - Guard\n";
    std::cout << "Move into a monster to attack it. Type 'help' for available commands.\n\n";

    std::string line;
    while (this->isRunning)
    {
        std::cout << "Turn " << this->turn << "\n> ";
        if (!std::getline(std::cin, line))
        {
            std::cout << "\n";
            break;
        }

        if (line.empty())
        {
            continue;
        }

        bool turnSpent = this->processCommand(line);

        if (turnSpent)
        {
            /* После хода игрока каждое существо в контейнере делает свой ход */
            if (this->isRunning)
            {
                this->monstersTakeTurns();
            }
            this->draw();
        }

        if (this->isRunning)
        {
            if (turnSpent)
            {
                ++this->turn;
            }
            std::cout << "\n";
        }
    }
}
