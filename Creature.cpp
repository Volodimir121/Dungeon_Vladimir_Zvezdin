#include "Creature.h"

#include <cctype>
#include <stdexcept>

Creature::Creature(int x, int y, int maxHealth, char symbol)
    : x(x), y(y), health(maxHealth), maxHealth(maxHealth), symbol(symbol)
{
    /* Инвариант проверяется в конструкторе (практика 11, типовая ошибка №2) */
    Creature::requireNonNegative(x, y);

    if (maxHealth <= 0)
    {
        throw std::invalid_argument("Максимальное здоровье существа должно быть положительным.");
    }
    if (!std::isprint(static_cast<unsigned char>(symbol)) || symbol == ' ')
    {
        throw std::invalid_argument("Символ существа должен быть печатным и не пробелом.");
    }
}

void Creature::requireNonNegative(int x, int y)
{
    if (x < 0 || y < 0)
    {
        throw std::invalid_argument("Координаты существа не могут быть отрицательными.");
    }
}

int  Creature::getX() const         { return this->x; }
int  Creature::getY() const         { return this->y; }
int  Creature::getHealth() const    { return this->health; }
int  Creature::getMaxHealth() const { return this->maxHealth; }
char Creature::getSymbol() const    { return this->symbol; }
bool Creature::isAlive() const      { return this->health > 0; }

void Creature::takeDamage(int damage)
{
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
     * параметром — константным указателем `Creature* const this`:
     *     Creature::takeDamage(&hero, damage);
     *
     * Внутри метода обращение `this->health` обращается к полю именно того
     * экземпляра, для которого был вызван метод.
     *
     * Практика 11, критерий 3: отрицательный урон отвергается (здоровье нельзя
     * «поднять» через урон), здоровье никогда не уходит ниже 0.
     */
    if (damage <= 0)
    {
        return;
    }

    if (damage >= this->health)
    {
        this->health = 0;
    }
    else
    {
        this->health -= damage;
    }
}

void Creature::setPosition(int newX, int newY)
{
    Creature::requireNonNegative(newX, newY);
    this->x = newX;
    this->y = newY;
}
