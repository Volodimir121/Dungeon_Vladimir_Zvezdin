#include "Game.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

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

    /* Датчик случайных чисел заполняется один раз: им пользуются генерация карты и монстры */
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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

    std::cout << "=== Dungeon Crawler ===\n";
    std::cout << "Select map mode:\n";
    std::cout << "  1. Generate map by size (30% walls, 67% floor, 3% gold)\n";
    std::cout << "  2. Load level from file (default: level.txt)\n";

    while (true)
    {
        std::cout << "Choice (1 or 2) [1]: ";

        std::string choice;
        if (!std::getline(std::cin, choice))
        {
            return 0;
        }

        if (choice == "q" || choice == "quit")
        {
            return 0;
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
                 * Монстры расставляются по символам 'M' из файла уровня.
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
            std::cout << "Please enter 1 or 2.\n\n";
        }
    }

    return 0;
}
