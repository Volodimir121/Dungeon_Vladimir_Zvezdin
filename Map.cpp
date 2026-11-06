#include "Map.h"
#include "Hero.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>

/* -------------------------------------------------------------------------- */
/*  Практика 10, задания 1–3: конструктор по размерам и деструктор (RAII)     */
/* -------------------------------------------------------------------------- */

Map::Map(int rows, int cols)
    : rows(rows), cols(cols), data(nullptr), startHeroX(1), startHeroY(1)
{
    if (this->rows < 3 || this->cols < 3)
    {
        throw std::invalid_argument("Размер карты должен быть не менее 3x3.");
    }

    std::size_t totalCells = static_cast<std::size_t>(this->rows) * static_cast<std::size_t>(this->cols);

    /*
     * Практика 10, задания 1 и 3:
     * Выделяем динамическую память под клетки в конструкторе и выводим сообщение.
     */
    this->data = new char[totalCells];
    std::memset(this->data, FLOOR, totalCells);

    /* Заполняем внешний периметр стенами по умолчанию */
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

    std::cout << "[Map] Конструктор: создана карта " << this->rows << "x" << this->cols
              << " (выделено " << totalCells << " байт по адресу "
              << static_cast<const void*>(this->data) << ")\n";
}

/* -------------------------------------------------------------------------- */
/*  Практика 10, задание 6: безопасная загрузка из файла (RAII)               */
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
        /* Пропускаем пустые строки в конце файла */
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

Map::Map(const std::vector<std::string>& lines, int* outHeroX, int* outHeroY)
    : Map(static_cast<int>(lines.size()), static_cast<int>(lines[0].size()))
{
    /*
     * Практика 10, задание 6:
     * К этому моменту целевой (делегированный) конструктор Map(rows, cols)
     * УЖЕ завершился и выделил память this->data.
     * По стандарту C++17, если в теле делегирующего конструктора возникнет
     * исключение (например, встретится недопустимый символ в файле),
     * компилятор АВТОМАТИЧЕСКИ вызовет деструктор ~Map(), который освободит
     * this->data, а сам объект Map создан не будет!
     * Таким образом, ни одного ручного delete[] вне ~Map() не требуется.
     */
    bool heroFound = false;

    for (int r = 0; r < this->rows; ++r)
    {
        for (int c = 0; c < this->cols; ++c)
        {
            char ch = lines[r][c];
            if (ch == HERO)
            {
                this->startHeroX = c;
                this->startHeroY = r;
                if (outHeroX != nullptr) *outHeroX = c;
                if (outHeroY != nullptr) *outHeroY = r;
                heroFound = true;
                this->setCell(c, r, FLOOR);
            }
            else if (ch == MONSTER)
            {
                /* Монстры переносятся на следующих практиках — оставляем клетку полом */
                this->setCell(c, r, FLOOR);
            }
            else if (ch == WALL || ch == FLOOR || ch == GOLD || ch == EXIT)
            {
                this->setCell(c, r, ch);
            }
            else
            {
                throw std::runtime_error(
                    std::string("Недопустимый символ '") + ch + "' в файле уровня.");
            }
        }
    }

    if (!heroFound)
    {
        for (int r = 1; r < this->rows - 1 && !heroFound; ++r)
        {
            for (int c = 1; c < this->cols - 1 && !heroFound; ++c)
            {
                if (this->getCell(c, r) == FLOOR)
                {
                    this->startHeroX = c;
                    this->startHeroY = r;
                    if (outHeroX != nullptr) *outHeroX = c;
                    if (outHeroY != nullptr) *outHeroY = r;
                    heroFound = true;
                }
            }
        }
    }
}

Map::Map(const std::string& filename, int* outHeroX, int* outHeroY)
    : Map(readLinesFromFile(filename), outHeroX, outHeroY)
{
    std::cout << "[Map] Уровень успешно загружен из файла '" << filename << "'.\n";
}

Map::~Map()
{
    /*
     * Практика 10, задания 2 и 3:
     * Единственное место во всей программе, где освобождается массив клеток.
     * Вызывается автоматически при выходе стекового объекта из области видимости
     * или при вызове delete для объекта в куче.
     */
    std::cout << "[Map] Деструктор: освобождается память карты "
              << this->rows << "x" << this->cols
              << " (адрес буфера " << static_cast<const void*>(this->data) << ")\n";

    delete[] this->data;
    this->data = nullptr;
}

/* -------------------------------------------------------------------------- */
/*  Практика 9, задания 2 и 5: методы работы с картой                         */
/* -------------------------------------------------------------------------- */

int Map::index(int x, int y) const
{
    /*
     * Практика 9, задание 5:
     * При вызове map.getCell(x, y) -> this->index(x, y) компилятор неявно передаёт
     * указатель `const Map* const this` на тот объект Map, у которого вызван метод.
     * Запись `this->cols` явно обращается к полю cols текущего объекта.
     */
    return y * this->cols + x;
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

void Map::setCell(int x, int y, char ch)
{
    if (!this->inBounds(x, y))
    {
        return;
    }
    this->data[this->index(x, y)] = ch;
}

bool Map::canEnter(int x, int y) const
{
    if (!this->inBounds(x, y))
    {
        return false;
    }
    return this->getCell(x, y) != WALL;
}

void Map::print(const Hero& hero) const
{
    if (this->data == nullptr)
    {
        return;
    }

    for (int r = 0; r < this->rows; ++r)
    {
        for (int c = 0; c < this->cols; ++c)
        {
            if (c == hero.x && r == hero.y)
            {
                std::cout << HERO;
            }
            else
            {
                std::cout << this->getCell(c, r);
            }
        }
        std::cout << '\n';
    }
}

void Map::generateProcedural(int* outHeroX, int* outHeroY)
{
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

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

    /* 2. Расставляем внутренние стены */
    int wallsPlaced = 0;
    int attempts = 0;
    while (wallsPlaced < innerWalls && attempts < innerCells * 4)
    {
        ++attempts;
        int r = 1 + std::rand() % (this->rows - 2);
        int c = 1 + std::rand() % (this->cols - 2);
        if (this->getCell(c, r) == FLOOR)
        {
            this->setCell(c, r, WALL);
            ++wallsPlaced;
        }
    }

    /* 3. Расставляем золото */
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

    /* 4. Выбираем стартовую клетку героя */
    int hx = 1;
    int hy = 1;
    this->setCell(hx, hy, FLOOR);
    this->startHeroX = hx;
    this->startHeroY = hy;
    if (outHeroX != nullptr) *outHeroX = hx;
    if (outHeroY != nullptr) *outHeroY = hy;

    /* 5. Размещаем выход в правом нижнем углу и гарантируем проходимый маршрут */
    int ex = this->cols - 2;
    int ey = this->rows - 2;
    int curX = hx;
    int curY = hy;
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
