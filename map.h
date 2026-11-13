#pragma once
#include <string>
#include <utility>
#include <vector>

/*
 * Класс Map — карта подземелья (хранение данных).
 *
 * Практика 9, задание 2: размеры, содержимое, getCell/setCell/canEnter.
 * Практика 10, задания 1–2, 6: динамическая память под клетки, деструктор (RAII),
 *   запрет копирования, безопасная загрузка из файла через делегирующий конструктор.
 *
 * Практика 11:
 *   — все поля закрыты;
 *   — класс ничего не выводит на экран (отрисовка вынесена в MapRenderer);
 *   — класс только хранит данные и отвечает на вопросы о них (принцип
 *     единственной ответственности).
 *
 * ИНВАРИАНТ КЛАССА (практика 11, критерий 1) — верно всегда, пока объект существует:
 *   1) rows >= 3 и cols >= 3;
 *   2) data указывает на непрерывный блок ровно rows * cols символов;
 *   3) каждая клетка содержит один из символов WALL, FLOOR, GOLD, EXIT
 *      (символы героя и монстров в данных не хранятся: их рисует MapRenderer);
 *   4) стартовая клетка героя (startHeroX, startHeroY) лежит в границах карты
 *      и не является стеной;
 *   5) все точки появления монстров лежат в границах карты.
 * Конструкторы устанавливают инвариант (иначе бросают исключение), а единственный
 * изменяющий публичный метод setCell отвергает всё, что его нарушает.
 */
class Map
{
public:
    /* Символы клеток */
    static constexpr char WALL  = '#';
    static constexpr char FLOOR = ' ';
    static constexpr char GOLD  = '$';
    static constexpr char EXIT  = 'E';

    /* Служебные символы файла уровня (в данных карты не хранятся) */
    static constexpr char HERO    = '@';
    static constexpr char MONSTER = 'M';

    /* Тег для конструктора с процедурной генерацией */
    struct Procedural {};

    /* Практика 10, задание 1: пустая карта с рамкой из стен */
    Map(int rows, int cols);

    /*
     * Карта, сгенерированная процедурно (30% стен, 3% золота, выход, гарантированный путь).
     * Делегирует базовому конструктору, поэтому RAII работает так же, как при загрузке из файла.
     */
    Map(int rows, int cols, Procedural);

    /*
     * Практика 10, задание 6: загрузка уровня из файла.
     * Если разбор упадёт, делегированный конструктор уже завершился, и C++
     * сам вызовет ~Map(); утечки памяти нет.
     */
    explicit Map(const std::string& filename);

    /* Практика 10, задание 2: освобождает память клеток (RAII) */
    ~Map();

    /* Практика 10, подсказка: владеем сырым указателем, копирование запрещено */
    Map(const Map&)            = delete;
    Map& operator=(const Map&) = delete;

    /* --- Геттеры: значения реально нужны Game и MapRenderer --- */
    int getRows() const;
    int getCols() const;
    int getStartHeroX() const;
    int getStartHeroY() const;

    /* Копия списка точек появления монстров (x, y); внутренний вектор наружу не отдаётся */
    std::vector<std::pair<int, int>> getMonsterSpawns() const;

    /* --- Методы работы с картой --- */

    /* Символ клетки; за границами карты возвращает WALL */
    char getCell(int x, int y) const;

    /*
     * Задать символ клетки. Возвращает false и ничего не меняет, если клетка вне карты,
     * символ не является символом клетки, либо запрошена стена на стартовой клетке героя.
     */
    bool setCell(int x, int y, char ch);

    bool inBounds(int x, int y) const;

    /* Внутри границ и не стена */
    bool canEnter(int x, int y) const;

private:
    int  index(int x, int y) const;
    void generateProcedural();

    /* Делегирующий конструктор для безопасной загрузки из файла (RAII) */
    explicit Map(const std::vector<std::string>& lines);

    static std::vector<std::string> readLinesFromFile(const std::string& filename);
    static bool isTileSymbol(char ch);

    int   rows;
    int   cols;
    char* data; /* data[y * cols + x] */
    int   startHeroX;
    int   startHeroY;
    std::vector<std::pair<int, int>> monsterSpawns;
};
