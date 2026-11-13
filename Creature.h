#pragma once
#include <string>

class Map;
class Creature;

/*
 * Практика 12, задания 5–6 и подсказка про связи между классами:
 * Интерфейс «мира», который существо видит во время своего хода.
 *
 * Существо не знает о классе Game: оно получает только этот узкий интерфейс
 * (карта для чтения, игрок, проверка занятости клетки, журнал сообщений).
 * Это ассоциация: мир передаётся в метод на время хода, существо им не владеет
 * и не хранит указатель на него. Благодаря интерфейсу монстры можно было бы
 * проверять без настоящей игры.
 */
class IWorld
{
public:
    virtual ~IWorld() = default;

    virtual const Map& getMap() const = 0;
    virtual Creature&  getPlayer() = 0;
    virtual bool       isCellOccupied(int x, int y) const = 0;
    virtual void       report(const std::string& message) = 0;
};

/*
 * Практика 12, задания 1–2: абстрактный базовый класс существа.
 *
 * ИНВАРИАНТ КЛАССА (практика 11, критерий 1) — верно для любого живого объекта:
 *   1) 0 <= health <= maxHealth, причём maxHealth > 0;
 *   2) координаты (x, y) неотрицательны;
 *   3) символ на карте — печатный и не пробел.
 * Конструктор проверяет все три пункта и бросает std::invalid_argument.
 * Изменить состояние снаружи можно только через takeDamage (держит пункт 1)
 * и через защищённый setPosition (держит пункт 2), его вызывают наследники.
 * Принадлежность координат конкретной карте проверяют наследники через
 * Map::canEnter перед вызовом setPosition.
 *
 * Класс абстрактный: чисто виртуальный takeTurn() не даёт создать объект Creature.
 * Деструктор виртуальный: иначе delete через указатель на базовый класс
 * не вызвал бы деструктор наследника (см. Game::runVirtualDestructorDemo).
 * Копирование запрещено: копия базового подобъекта «срезала» бы наследника.
 */
class Creature
{
public:
    virtual ~Creature() = default;

    Creature(const Creature&)            = delete;
    Creature& operator=(const Creature&) = delete;

    /* Геттеры: значения нужны игре, отрисовке и монстрам */
    int  getX() const;
    int  getY() const;
    int  getHealth() const;
    int  getMaxHealth() const;
    char getSymbol() const;
    bool isAlive() const;

    /* Получить урон: здоровье уменьшается, но не опускается ниже 0 */
    void takeDamage(int damage);

    /*
     * Практика 12, задание 2: чисто виртуальный метод хода.
     * Каждый наследник реализует своё поведение; игровой цикл вызывает
     * его одинаково для всех существ.
     */
    virtual void takeTurn(IWorld& world) = 0;

    /* Имя для сообщений в журнале игры */
    virtual std::string getName() const = 0;

protected:
    Creature(int x, int y, int maxHealth, char symbol);

    /* Сменить позицию. Вызывающий обязан сам убедиться, что клетка проходима. */
    void setPosition(int x, int y);

private:
    /* Общая проверка координат для конструктора и setPosition */
    static void requireNonNegative(int x, int y);

    int  x;
    int  y;
    int  health;
    int  maxHealth;
    char symbol;
};
