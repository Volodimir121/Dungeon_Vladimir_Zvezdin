#include "Hero.h"
#include "Map.h"

#include <iomanip>
#include <iostream>
#include <limits>

/* -------------------------------------------------------------------------- */
/*  Практика 10, задание 5: конструкторы и деструктор класса Hero             */
/* -------------------------------------------------------------------------- */

Hero::Hero()
    : Creature(1, 1, 100, '@'), gold(0)
{
    std::cout << "[Hero] Конструктор по умолчанию: герой создан в точке ("
              << this->getX() << ", " << this->getY() << "), HP=" << this->getHealth()
              << ", Gold=" << this->gold << "\n";
}

Hero::Hero(int startX, int startY, int startHealth, unsigned int startGold)
    : Creature(startX, startY, startHealth, '@'), gold(startGold)
{
    std::cout << "[Hero] Конструктор с параметрами: герой создан в точке ("
              << this->getX() << ", " << this->getY() << "), HP=" << this->getHealth()
              << ", Gold=" << this->gold << "\n";
}

Hero::~Hero()
{
    std::cout << "[Hero] Деструктор: герой в точке ("
              << this->getX() << ", " << this->getY() << ") уничтожен.\n";
}

/* -------------------------------------------------------------------------- */
/*  Методы поведения                                                          */
/* -------------------------------------------------------------------------- */

unsigned int Hero::getGold() const
{
    return this->gold;
}

bool Hero::move(int dx, int dy, const Map& map)
{
    int nextX = this->getX() + dx;
    int nextY = this->getY() + dy;

    /*
     * Практика 9, подсказка:
     * «Класс карты и класс героя не должны знать друг о друге больше необходимого.
     *  Подумайте, кто кому задаёт вопрос при попытке шага.»
     *
     * Герой задаёт карте вопрос через публичный метод map.canEnter(nextX, nextY).
     * Инвариант: позиция меняется только после положительного ответа карты.
     */
    if (!map.canEnter(nextX, nextY))
    {
        return false;
    }

    this->setPosition(nextX, nextY);
    return true;
}

void Hero::addGold(unsigned int amount)
{
    if (amount > std::numeric_limits<unsigned int>::max() - this->gold)
    {
        this->gold = std::numeric_limits<unsigned int>::max();
    }
    else
    {
        this->gold += amount;
    }
}

void Hero::printStatus() const
{
    double percent = (this->getMaxHealth() > 0)
        ? (static_cast<double>(this->getHealth()) / static_cast<double>(this->getMaxHealth())) * 100.0
        : 0.0;

    std::cout << "=== Состояние героя ===\n";
    std::cout << "Позиция : (" << this->getX() << ", " << this->getY() << ")\n";
    std::cout << "Здоровье: " << this->getHealth() << "/" << this->getMaxHealth()
              << " (" << std::fixed << std::setprecision(1) << percent << "%)\n";
    std::cout << "Золото  : " << this->gold << "\n";
}

void Hero::takeTurn(IWorld& /*world*/)
{
    /* Ход героя выполняет игрок командой (Game::processCommand). */
}

std::string Hero::getName() const
{
    return "Hero";
}
