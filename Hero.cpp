#include "Hero.h"
#include "Map.h"

#include <iomanip>
#include <iostream>

/* -------------------------------------------------------------------------- */
/*  Практика 10, задание 5: конструкторы и деструктор класса Hero             */
/* -------------------------------------------------------------------------- */

Hero::Hero()
    : x(1), y(1), health(100), maxHealth(100), gold(0)
{
    std::cout << "[Hero] Конструктор по умолчанию: герой создан в точке ("
              << this->x << ", " << this->y << "), HP=" << this->health
              << ", Gold=" << this->gold << "\n";
}

Hero::Hero(int startX, int startY, int startHealth, unsigned int startGold)
    : x(startX),
      y(startY),
      health(startHealth > 0 ? startHealth : 100),
      maxHealth(startHealth > 0 ? startHealth : 100),
      gold(startGold)
{
    std::cout << "[Hero] Конструктор с параметрами: герой создан в точке ("
              << this->x << ", " << this->y << "), HP=" << this->health
              << ", Gold=" << this->gold << "\n";
}

Hero::~Hero()
{
    std::cout << "[Hero] Деструктор: герой в точке ("
              << this->x << ", " << this->y << ") уничтожен.\n";
}

/* -------------------------------------------------------------------------- */
/*  Практика 9, задания 3 и 5: методы класса Hero и указатель this            */
/* -------------------------------------------------------------------------- */

bool Hero::move(int dx, int dy, const Map& map)
{
    int nextX = this->x + dx;
    int nextY = this->y + dy;

    /*
     * Практика 9, подсказка:
     * «Класс карты и класс героя не должны знать друг о друге больше необходимого.
     *  Подумайте, кто кому задаёт вопрос при попытке шага.»
     *
     * Герой не лезет во внутренний массив клеток карты, а задаёт карте вопрос
     * через публичный метод map.canEnter(nextX, nextY).
     */
    if (!map.canEnter(nextX, nextY))
    {
        return false;
    }

    this->x = nextX;
    this->y = nextY;
    return true;
}

void Hero::takeDamage(int damage)
{
    if (damage < 0)
    {
        return;
    }

    /*
     * Практика 9, задание 5 и критерий 4:
     * ЧЕМ ВЫЗОВ МЕТОДА ОТЛИЧАЕТСЯ ОТ ВЫЗОВА СВОБОДНОЙ ФУНКЦИИ В C:
     *
     * В старой версии на C функция принимала указатель на структуру явно:
     *     hero_damage(&hero, damage);
     *
     * В C++ метод вызывается через объект:
     *     hero.takeDamage(damage);
     * При этом компилятор НЕЯВНО передаёт адрес объекта (&hero) скрытым первым
     * параметром — константным указателем `Hero* const this`:
     *     Hero::takeDamage(&hero, damage);
     *
     * Внутри метода обращение `this->health` обращается к полю именно того
     * экземпляра Hero, для которого был вызван метод.
     */
    this->health -= damage;
    if (this->health < 0)
    {
        this->health = 0;
    }
}

void Hero::addGold(unsigned int amount)
{
    this->gold += amount;
}

void Hero::printStatus() const
{
    double percent = (this->maxHealth > 0)
        ? (static_cast<double>(this->health) / static_cast<double>(this->maxHealth)) * 100.0
        : 0.0;

    std::cout << "=== Состояние героя ===\n";
    std::cout << "Позиция : (" << this->x << ", " << this->y << ")\n";
    std::cout << "Здоровье: " << this->health << "/" << this->maxHealth
              << " (" << std::fixed << std::setprecision(1) << percent << "%)\n";
    std::cout << "Золото  : " << this->gold << "\n";
}

void Hero::drawOn(const Map& map) const
{
    /*
     * Практика 9, задание 5:
     * «Найдите в своём коде место, где методу нужно передать сам объект,
     *  и используйте this явно. Объясните в комментарии, что происходит при вызове метода.»
     *
     * Здесь метод Hero::drawOn должен передать САМ текущий объект Hero в метод
     * отрисовки карты Map::print(const Hero& hero).
     * Так как `this` — это указатель (const Hero* const) на текущий объект,
     * для которого был вызван метод hero.drawOn(map), разыменование `*this`
     * даёт ссылку на сам этот объект (const Hero&).
     *
     * Что происходит при вызове hero.drawOn(map):
     * 1. Компилятор вызывает Hero::drawOn(&hero, map), записывая &hero в указатель `this`.
     * 2. Внутри метода мы разыменовываем `*this` и передаём сам объект героя
     *    в вызов map.print(*this), который транслируется в Map::print(&map, *this).
     */
    map.print(*this);
}

bool Hero::isAlive() const
{
    return this->health > 0;
}
