#include "Map.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>

/* -------------------------------------------------------------------------- */
/*  Конструкторы и деструктор (практика 10, RAII)                             */
/* -------------------------------------------------------------------------- */

Map::Map(int rows, int cols)
    : rows(rows), cols(cols), data(nullptr), startHeroX(1), startHeroY(1)
{
    /* Инвариант п.1 проверяется в конструкторе до выделения памяти */
    if (this->rows < 3 || this->cols < 3)
    {
        throw std::invalid_argument("Размер карты должен быть не менее 3x3.");
    }

    std::size_t totalCells = static_cast<std::size_t>(this->rows) * static_cast<std::size_t>(this->cols);

    this->data = new char[totalCells];
    std::memset(this->data, FLOOR, totalCells);

    /* Внешний периметр — стены по умолчанию. Клетка (1, 1) внутри, так что инвариант п.4 выполнен. */
    for (int r = 0; r < this->rows; ++r)
    {
        for (int c = 0; c < this->cols; ++c)
        {
            if (r == 0 || r == this->rows - 1 || c == 0 || c == this->cols - 1)
            {
                this->data[r * this->cols + c] = WALL;
            }
        }
    }
}

Map::Map(int rows, int cols, Procedural)
    : Map(rows, cols)
{
    this->generateProcedural();
}

Map::~Map()
{
    /*
     * Единственное место, где освобождается массив клеток.
     * Практика 11: никакого вывода на экран, поэтому момент уничтожения
     * карты в демонстрациях показывают вспомогательные объекты-«трассировщики».
     */
    delete[] this->data;
    this->data = nullptr;
}

/* -------------------------------------------------------------------------- */
/*  Загрузка из файла                                                         */
/* -------------------------------------------------------------------------- */

std::vector<std::string> Map::readLinesFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Не удалось открыть файл уровня: " + filename);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        /* Пропускаем пустые строки */
        if (!line.empty())
        {
            lines.push_back(line);
        }
    }

    if (lines.empty())
    {
        throw std::runtime_error("Файл уровня пуст: " + filename);
    }

    std::size_t expectedCols = lines[0].size();
    if (lines.size() < 3 || expectedCols < 3)
    {
        throw std::runtime_error("Размер карты в файле меньше минимального (3x3): " + filename);
    }

    for (std::size_t r = 0; r < lines.size(); ++r)
    {
        if (lines[r].size() != expectedCols)
        {
            throw std::runtime_error("Несовпадение длины строк в файле уровня: " + filename);
        }
    }

    return lines;
}

Map::Map(const std::vector<std::string>& lines)
    : Map(static_cast<int>(lines.size()), static_cast<int>(lines[0].size()))
{
    /*
     * Делегированный конструктор Map(rows, cols) уже завершился и выделил память.
     * Если ниже будет брошено исключение, C++ сам вызовет ~Map() и освободит data.
     */

    /* Проход 1: проверяем символы, ищем героя и запасную свободную клетку */
    int heroX = -1;
    int heroY = -1;
    int freeX = -1;
    int freeY = -1;

    for (int r = 0; r < this->rows; ++r)
    {
        for (int c = 0; c < this->cols; ++c)
        {
            char ch = lines[r][c];
            if (!Map::isTileSymbol(ch) && ch != HERO && ch != MONSTER)
            {
                throw std::runtime_error(
                    std::string("Недопустимый символ '") + ch + "' в файле уровня.");
            }
            if (ch == HERO && heroX < 0)
            {
                heroX = c;
                heroY = r;
            }
            if (ch == FLOOR && freeX < 0 &&
                r >= 1 && r < this->rows - 1 && c >= 1 && c < this->cols - 1)
            {
                freeX = c;
                freeY = r;
            }
        }
    }

    if (heroX < 0)
    {
        heroX = freeX;
        heroY = freeY;
    }
    if (heroX < 0)
    {
        throw std::runtime_error("В файле уровня нет свободной клетки для героя.");
    }

    /* Стартовая клетка задаётся ДО раскладки клеток, чтобы setCell её не блокировал */
    this->startHeroX = heroX;
    this->startHeroY = heroY;

    /* Проход 2: заполняем карту */
    for (int r = 0; r < this->rows; ++r)
    {
        for (int c = 0; c < this->cols; ++c)
        {
            char ch = lines[r][c];
            if (ch == HERO)
            {
                this->setCell(c, r, FLOOR);
            }
            else if (ch == MONSTER)
            {
                this->setCell(c, r, FLOOR);
                this->monsterSpawns.emplace_back(c, r);
            }
            else
            {
                this->setCell(c, r, ch);
            }
        }
    }
}

Map::Map(const std::string& filename)
    : Map(readLinesFromFile(filename))
{
}

/* -------------------------------------------------------------------------- */
/*  Геттеры и методы работы с картой                                          */
/* -------------------------------------------------------------------------- */

int Map::getRows() const       { return this->rows; }
int Map::getCols() const       { return this->cols; }
int Map::getStartHeroX() const { return this->startHeroX; }
int Map::getStartHeroY() const { return this->startHeroY; }

std::vector<std::pair<int, int>> Map::getMonsterSpawns() const
{
    return this->monsterSpawns;
}

int Map::index(int x, int y) const
{
    /*
     * При вызове map.getCell(x, y) -> this->index(x, y) компилятор неявно передаёт
     * указатель `const Map* const this` на объект, у которого вызван метод.
     */
    return y * this->cols + x;
}

bool Map::isTileSymbol(char ch)
{
    return ch == WALL || ch == FLOOR || ch == GOLD || ch == EXIT;
}

bool Map::inBounds(int x, int y) const
{
    return (this->data != nullptr &&
            x >= 0 && x < this->cols &&
            y >= 0 && y < this->rows);
}

char Map::getCell(int x, int y) const
{
    if (!this->inBounds(x, y))
    {
        return WALL;
    }
    return this->data[this->index(x, y)];
}

bool Map::setCell(int x, int y, char ch)
{
    /* Каждая проверка защищает пункт инварианта: п.2, п.3, п.4 */
    if (!this->inBounds(x, y))
    {
        return false;
    }
    if (!Map::isTileSymbol(ch))
    {
        return false;
    }
    if (ch == WALL && x == this->startHeroX && y == this->startHeroY)
    {
        return false;
    }

    this->data[this->index(x, y)] = ch;
    return true;
}

bool Map::canEnter(int x, int y) const
{
    if (!this->inBounds(x, y))
    {
        return false;
    }
    return this->getCell(x, y) != WALL;
}

/* -------------------------------------------------------------------------- */
/*  Процедурная генерация (датчик случайных чисел заполняется в main)         */
/* -------------------------------------------------------------------------- */

void Map::generateProcedural()
{
    this->monsterSpawns.clear();

    /* 1. Очищаем внутреннюю область и строим стены по границе */
    int borderWalls = 0;
    for (int r = 0; r < this->rows; ++r)
    {
        for (int c = 0; c < this->cols; ++c)
        {
            if (r == 0 || r == this->rows - 1 || c == 0 || c == this->cols - 1)
            {
                this->setCell(c, r, WALL);
                ++borderWalls;
            }
            else
            {
                this->setCell(c, r, FLOOR);
            }
        }
    }

    int totalCells = this->rows * this->cols;
    int targetWalls = (totalCells * 30 + 50) / 100;
    int targetGold  = (totalCells * 3 + 50) / 100;
    if (targetGold < 1)
    {
        targetGold = 1;
    }

    int innerCells = (this->rows - 2) * (this->cols - 2);
    int innerWalls = (targetWalls > borderWalls) ? (targetWalls - borderWalls) : 0;
    if (innerWalls > innerCells / 3)
    {
        innerWalls = innerCells / 3;
    }

    /* 2. Внутренние стены (setCell сама не даст поставить стену на стартовую клетку) */
    int wallsPlaced = 0;
    int attempts = 0;
    while (wallsPlaced < innerWalls && attempts < innerCells * 4)
    {
        ++attempts;
        int r = 1 + std::rand() % (this->rows - 2);
        int c = 1 + std::rand() % (this->cols - 2);
        if (this->getCell(c, r) == FLOOR && this->setCell(c, r, WALL))
        {
            ++wallsPlaced;
        }
    }

    /* 3. Золото */
    int goldPlaced = 0;
    attempts = 0;
    while (goldPlaced < targetGold && attempts < innerCells * 4)
    {
        ++attempts;
        int r = 1 + std::rand() % (this->rows - 2);
        int c = 1 + std::rand() % (this->cols - 2);
        if (this->getCell(c, r) == FLOOR)
        {
            this->setCell(c, r, GOLD);
            ++goldPlaced;
        }
    }

    /* 4. Стартовая клетка героя */
    this->startHeroX = 1;
    this->startHeroY = 1;
    this->setCell(this->startHeroX, this->startHeroY, FLOOR);

    /* 5. Выход в правом нижнем углу и гарантированно проходимый маршрут */
    int ex = this->cols - 2;
    int ey = this->rows - 2;
    int curX = this->startHeroX;
    int curY = this->startHeroY;
    while (curX != ex)
    {
        curX += (ex > curX) ? 1 : -1;
        if (this->getCell(curX, curY) == WALL)
        {
            this->setCell(curX, curY, FLOOR);
        }
    }
    while (curY != ey)
    {
        curY += (ey > curY) ? 1 : -1;
        if (this->getCell(curX, curY) == WALL)
        {
            this->setCell(curX, curY, FLOOR);
        }
    }
    this->setCell(ex, ey, EXIT);
}
