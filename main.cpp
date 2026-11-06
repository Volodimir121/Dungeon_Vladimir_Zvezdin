#include "Game.h"
#include "Hero.h"
#include "Map.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * Практика 10, задания 3, 4, 5, 6:
 * Демонстрация времени жизни объектов (стек, вложенный блок, куча, ошибка загрузки файла).
 *
 * ПОРЯДОК ВЫЗОВОВ КОНСТРУКТОРОВ И ДЕСТРУКТОРОВ (Практика 10, критерий 2):
 * 1. Объекты на стеке конструируются сверху вниз по мере выполнения кода,
 *    а уничтожаются АВТОМАТИЧЕСКИ при выходе из своей области видимости `{ ... }`
 *    в порядке, СТРОГО ОБРАТНОМ порядку их создания (правило LIFO — стек):
 *      - Конструктор stackMap (4x8)
 *      - Конструктор defaultHero (по умолчанию)
 *      - Конструктор paramHero (с параметрами)
 *      ...
 *      При выходе из функции:
 *      - Деструктор paramHero
 *      - Деструктор defaultHero
 *      - Деструктор stackMap (4x8)
 *
 * 2. Объект внутри вложенного блока `{ Map blockMap(3, 5); }` уничтожается
 *    немедленно при достижении закрывающей скобки `}` этого блока, задолго до
 *    завершения всей функции.
 *
 * 3. Объект в куче (`new Map(5, 6)`) создаётся динамически и НЕ уничтожается
 *    сам при выходе из блока — для него программист обязан явно вызвать `delete`,
 *    который вызывает `~Map()` и освобождает память.
 *
 * 4. При ошибке чтения файла уровня после выделения памяти делегированным
 *    конструктором C++ автоматически вызывает деструктор `~Map()` во время
 *    раскрутки стека (RAII), поэтому утечки памяти не происходит, а сам
 *    объект карты не создаётся.
 */
static void demonstrateObjectLifetimes()
{
    std::cout << "================================================================\n";
    std::cout << "  ПРАКТИКА 10: Демонстрация конструкторов, деструкторов и RAII  \n";
    std::cout << "================================================================\n\n";

    std::cout << "--- 1. Создание объектов на стеке функции ---\n";
    Map  stackMap(4, 8);
    Hero defaultHero;                  /* Конструктор по умолчанию (Практика 10, зад. 5) */
    Hero paramHero(2, 2, 85, 10);      /* Конструктор с параметрами (Практика 10, зад. 5) */

    std::cout << "\n--- 2. Создание объекта Map внутри вложенного блока { ... } (Практика 10, зад. 4) ---\n";
    {
        std::cout << "  [Вход в блок]\n";
        Map blockMap(3, 5);
        std::cout << "  [Работа с blockMap внутри блока: символ (0,0) = '"
                  << blockMap.getCell(0, 0) << "']\n";
        std::cout << "  [Выход из блока — сейчас автоматически вызовется ~Map() для blockMap]\n";
    }
    std::cout << "  [Блок завершён: blockMap уже уничтожена]\n";

    std::cout << "\n--- 3. Создание объекта Map в куче (new / delete) (Практика 10, зад. 3) ---\n";
    Map* heapMap = new Map(5, 6);
    std::cout << "  [Объект 5x6 в куче создан; удаляем его явно через delete]\n";
    delete heapMap;
    heapMap = nullptr;

    std::cout << "\n--- 4. Проверка RAII при ошибке в файле уровня (Практика 10, зад. 6) ---\n";
    const char* brokenFile = "broken_level_temp.txt";
    {
        /* Создаём временный файл корректного размера 4x6, но с недопустимым символом '?' */
        std::ofstream badOut(brokenFile);
        badOut << "######\n"
               << "#@   #\n"
               << "# ? E#\n"
               << "######\n";
    }
    try
    {
        std::cout << "  [Попытка загрузить повреждённый файл '" << brokenFile << "'...]\n";
        Map brokenMap(brokenFile);
        std::cout << "  [Эта строка никогда не выполнится]\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "  [Перехвачено исключение]: " << ex.what() << "\n";
        std::cout << "  [Объект brokenMap не создан, а выделенная под 4x6 память уже автоматически освобождена ~Map()!]\n";
    }
    std::remove(brokenFile);

    std::cout << "\n--- 5. Завершение функции demonstrateObjectLifetimes() ---\n";
    std::cout << "  [Сейчас стековые объекты уничтожатся в обратном порядке: paramHero -> defaultHero -> stackMap]\n";
}

static int parsePositiveInt(const std::string& s, int defaultVal, int minVal, int maxVal)
{
    if (s.empty())
    {
        return defaultVal;
    }
    try
    {
        int val = std::stoi(s);
        if (val >= minVal && val <= maxVal)
        {
            return val;
        }
    }
    catch (const std::exception&)
    {
    }
    std::cout << "Invalid value, using default " << defaultVal << ".\n";
    return defaultVal;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    /* Если передан путь к файлу в аргументах командной строки */
    if (argc > 1)
    {
        try
        {
            Game game(argv[1]);
            game.run();
            return 0;
        }
        catch (const std::exception& ex)
        {
            std::cout << "Error loading file '" << argv[1] << "': " << ex.what() << "\n";
            return 1;
        }
    }

    /* Наглядная демонстрация Практики 10 (стек, куча, блок, ошибка файла, порядок конструкторов/деструкторов) */
    demonstrateObjectLifetimes();
    std::cout << "================================================================\n\n";

    while (true)
    {
        std::cout << "=== Dungeon Crawler (C++17: Practices 9 & 10) ===\n";
        std::cout << "Select map mode:\n";
        std::cout << "  1. Generate map by size (30% walls, 67% floor, 3% gold)\n";
        std::cout << "  2. Load level from file (default: level.txt)\n";
        std::cout << "  3. Repeat Practice 10 lifetime & RAII demonstration\n";
        std::cout << "  q. Quit\n";
        std::cout << "Choice (1, 2, 3, q) [1]: ";

        std::string choice;
        if (!std::getline(std::cin, choice))
        {
            return 0;
        }

        if (choice == "q" || choice == "quit")
        {
            return 0;
        }

        if (choice == "3")
        {
            std::cout << "\n";
            demonstrateObjectLifetimes();
            std::cout << "================================================================\n\n";
            continue;
        }

        if (choice.empty() || choice == "1")
        {
            std::string input;
            std::cout << "Enter rows [default 10]: ";
            if (!std::getline(std::cin, input)) return 0;
            int rows = parsePositiveInt(input, 10, 4, 100);

            std::cout << "Enter cols [default 20]: ";
            if (!std::getline(std::cin, input)) return 0;
            int cols = parsePositiveInt(input, 20, 4, 100);

            try
            {
                Game game(rows, cols);
                game.run();
                break;
            }
            catch (const std::exception& ex)
            {
                std::cout << "Error creating game: " << ex.what() << "\n\n";
            }
        }
        else if (choice == "2")
        {
            std::cout << "Enter level filename [default: level.txt]: ";
            std::string filename;
            if (!std::getline(std::cin, filename)) return 0;
            if (filename.empty())
            {
                filename = "level.txt";
            }

            try
            {
                /*
                 * Практика 10, задание 6:
                 * Если в файле ошибка, конструктор Map выбросит исключение,
                 * деструктор ~Map() автоматически освободит буфер (если он уже был выделен),
                 * объект Game создан не будет, и управление перейдёт в блок catch.
                 */
                Game game(filename);
                game.run();
                break;
            }
            catch (const std::exception& ex)
            {
                std::cout << "Error loading level '" << filename << "': " << ex.what() << "\n";
                std::cout << "Please try again.\n\n";
            }
        }
        else
        {
            std::cout << "Please enter 1, 2, 3, or q.\n\n";
        }
    }

    return 0;
}
