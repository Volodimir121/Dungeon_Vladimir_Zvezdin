#include "Game.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>

Game::Game(int rows, int cols)
    : map(rows, cols),   /* 1. Конструируется Map (выделяется память в куче) */
      hero(),            /* 2. Конструируется Hero конструктором по умолчанию (Практика 10, зад. 5) */
      turn(1),
      isRunning(true)
{
    int startX = 1;
    int startY = 1;
    this->map.generateProcedural(&startX, &startY);
    this->hero.x = startX;
    this->hero.y = startY;
}

Game::Game(const std::string& filename)
    : map(filename),     /* 1. Загружается Map из файла; при ошибке бросит исключение,
                               деструктор ~Map() автоматически освободит память,
                               и объект Game создан не будет (Практика 10, зад. 6) */
      hero(map.startHeroX, map.startHeroY, 100, 0), /* 2. Конструируется Hero конструктором с параметрами (Практика 10, зад. 5) */
      turn(1),
      isRunning(true)
{
}

Game::~Game()
{
    /*
     * Практика 10, критерий 2:
     * Порядок вызовов конструкторов и деструкторов:
     * При создании Game сначала вызывается конструктор поля `map`, затем поля `hero`.
     * При уничтожении Game сначала выполняется тело ~Game(), затем АВТОМАТИЧЕСКИ
     * в обратном порядке вызываются деструкторы полей: сначала ~Hero(), затем ~Map().
     * Никакого ручного освобождения памяти (free / delete) здесь писать не нужно (RAII).
     */
    std::cout << "[Game] Деструктор: игра завершена, далее автоматически уничтожатся Hero и Map.\n";
}

void Game::checkCurrentCell()
{
    char cell = this->map.getCell(this->hero.x, this->hero.y);
    if (cell == Map::GOLD)
    {
        this->hero.addGold(1);
        this->map.setCell(this->hero.x, this->hero.y, Map::FLOOR);
        std::cout << "You found gold! Total gold: " << this->hero.gold << "\n";
    }
    else if (cell == Map::EXIT)
    {
        std::cout << "Congratulations! You reached the exit!\n";
        std::cout << "Total Gold: " << this->hero.gold << " | Turns: " << this->turn << "\n";
        this->isRunning = false;
    }
}

void Game::moveHero(int dx, int dy, const std::string& dirName)
{
    int targetX = this->hero.x + dx;
    int targetY = this->hero.y + dy;

    if (!this->map.inBounds(targetX, targetY))
    {
        std::cout << "You can't go " << dirName << " - boundary blocks the way.\n";
        return;
    }

    /*
     * Практика 9: вызываем метод объекта hero.move(dx, dy, map).
     * Внутри hero.move герой спрашивает у карты map.canEnter(targetX, targetY).
     */
    if (!this->hero.move(dx, dy, this->map))
    {
        std::cout << "You can't go " << dirName << " - a wall blocks the way.\n";
        return;
    }

    std::cout << "You went " << dirName << ".\n";
    this->checkCurrentCell();

    /* Отрисовываем карту после перемещения с помощью метода, использующего *this */
    this->hero.drawOn(this->map);
}

void Game::printHelp() const
{
    std::cout << "Available commands (name, short form):\n"
              << "  north     (n)  - go north\n"
              << "  south     (s)  - go south\n"
              << "  east      (e)  - go east\n"
              << "  west      (w)  - go west\n"
              << "  go <dir>       - move in direction (e.g. go north)\n"
              << "  look      (l)  - redraw the dungeon map\n"
              << "  status    (st) - show hero status\n"
              << "  hit <N>        - take N damage (test Hero::takeDamage)\n"
              << "  test           - demonstrate stack, heap, and scope block (Practice 10)\n"
              << "  help      (h)  - show this help\n"
              << "  quit      (q)  - exit the game\n";
}

/*
 * Практика 10, задания 3 и 4:
 * Создание объектов на стеке, внутри вложенного блока { ... } и в куче (new / delete).
 *
 * ПОРЯДОК ВЫЗОВОВ КОНСТРУКТОРОВ И ДЕСТРУКТОРОВ (Практика 10, критерий 2):
 * 1. Стековые объекты (stackMap, stackHero) уничтожаются автоматически при выходе
 *    из функции в порядке, обратном порядку их создания (сначала ~Hero, затем ~Map).
 * 2. Объект внутри вложенного блока { Map blockMap(3, 5); } уничтожается автоматически
 *    сразу при выходе из этого блока '}', до завершения функции.
 * 3. Объект в куче (new Map(5, 6)) не уничтожается автоматически — он удаляется
 *    явно оператором delete.
 */
void Game::runLifetimeDemo()
{
    std::cout << "--- 1. Stack objects ---\n";
    Map  stackMap(4, 8);
    Hero stackHero(2, 2, 85, 10);

    std::cout << "\n--- 2. Object inside nested block { ... } ---\n";
    {
        Map blockMap(3, 5);
    }

    std::cout << "\n--- 3. Object on the heap (new / delete) ---\n";
    Map* heapMap = new Map(5, 6);
    delete heapMap;
    heapMap = nullptr;

    std::cout << "\n--- 4. Leaving function (stack objects destroyed in reverse order) ---\n";
}

void Game::processCommand(const std::string& rawLine)
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
        return;
    }

    std::string cmd = words[0];
    if (cmd == "go")
    {
        if (words.size() < 2)
        {
            std::cout << "Go where? Example: go north\n";
            return;
        }
        cmd = words[1];
        if (cmd != "north" && cmd != "n" &&
            cmd != "south" && cmd != "s" &&
            cmd != "east"  && cmd != "e" &&
            cmd != "west"  && cmd != "w")
        {
            std::cout << "You can't go '" << cmd << "'.\n";
            return;
        }
    }

    if (cmd == "north" || cmd == "n")
    {
        this->moveHero(0, -1, "north");
    }
    else if (cmd == "south" || cmd == "s")
    {
        this->moveHero(0, 1, "south");
    }
    else if (cmd == "east" || cmd == "e")
    {
        this->moveHero(1, 0, "east");
    }
    else if (cmd == "west" || cmd == "w")
    {
        this->moveHero(-1, 0, "west");
    }
    else if (cmd == "look" || cmd == "l")
    {
        std::cout << "You look around the dungeon.\n";
        /*
         * Практика 9, задание 5:
         * Вызываем метод hero.drawOn(map), внутри которого герой передаёт
         * сам себя (*this) в метод map.print(*this).
         */
        this->hero.drawOn(this->map);
    }
    else if (cmd == "status" || cmd == "st")
    {
        this->hero.printStatus();
    }
    else if (cmd == "hit")
    {
        if (words.size() < 2)
        {
            std::cout << "Usage: hit <damage>\n";
            return;
        }
        try
        {
            int dmg = std::stoi(words[1]);
            if (dmg < 0)
            {
                std::cout << "Damage must be non-negative.\n";
                return;
            }
            this->hero.takeDamage(dmg);
            std::cout << "You took " << dmg << " damage. Health: "
                      << this->hero.health << "/" << this->hero.maxHealth << "\n";
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
}

void Game::run()
{
    std::cout << "\nWelcome to the Dungeon!\n";
    this->hero.drawOn(this->map);
    std::cout << "\nSymbols: @ - Player, $ - Gold, E - Exit, # - Wall\n";
    std::cout << "Type 'help' for available commands.\n\n";

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

        this->processCommand(line);

        if (this->isRunning)
        {
            ++this->turn;
            std::cout << "\n";
        }
    }
}
