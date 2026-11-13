#include "Monster.h"
#include "Map.h"

#include <cstdlib>
#include <stdexcept>

namespace
{
int sign(int value)
{
    return (value > 0) - (value < 0);
}
}

/* -------------------------------------------------------------------------- */
/*  Monster                                                                   */
/* -------------------------------------------------------------------------- */

Monster::Monster(int x, int y, int maxHealth, char symbol, int attackDamage)
    : Creature(x, y, maxHealth, symbol), attackDamage(attackDamage)
{
    /* Инвариант проверяется в конструкторе; счётчик растёт только после успешной проверки */
    if (this->attackDamage < 0)
    {
        throw std::invalid_argument("Урон монстра не может быть отрицательным.");
    }
    ++Monster::totalCreated;
}

int Monster::getTotalCreated()
{
    return Monster::totalCreated;
}

bool Monster::attackIfAdjacent(IWorld& world)
{
    Creature& player = world.getPlayer();
    if (!player.isAlive())
    {
        return false;
    }

    int distance = std::abs(player.getX() - this->getX()) + std::abs(player.getY() - this->getY());
    if (distance != 1)
    {
        return false;
    }

    player.takeDamage(this->attackDamage);
    world.report(this->getName() + " hits you for " + std::to_string(this->attackDamage) + " damage.");
    return true;
}

bool Monster::tryStep(int dx, int dy, IWorld& world)
{
    int nextX = this->getX() + dx;
    int nextY = this->getY() + dy;

    if (!world.getMap().canEnter(nextX, nextY))
    {
        return false;
    }
    if (world.isCellOccupied(nextX, nextY))
    {
        return false;
    }

    this->setPosition(nextX, nextY);
    return true;
}

/* -------------------------------------------------------------------------- */
/*  Zombie                                                                    */
/* -------------------------------------------------------------------------- */

Zombie::Zombie(int x, int y)
    : Monster(x, y, 40, 'Z', 10)
{
}

void Zombie::takeTurn(IWorld& world)
{
    if (this->attackIfAdjacent(world))
    {
        return;
    }

    const Creature& player = world.getPlayer();
    int dx = player.getX() - this->getX();
    int dy = player.getY() - this->getY();

    if (std::abs(dx) + std::abs(dy) > VISION_RADIUS)
    {
        return;
    }

    int stepX = sign(dx);
    int stepY = sign(dy);

    /* Сначала по оси с большим расстоянием, при препятствии — по другой */
    if (std::abs(dx) >= std::abs(dy))
    {
        if (stepX != 0 && this->tryStep(stepX, 0, world)) return;
        if (stepY != 0) this->tryStep(0, stepY, world);
    }
    else
    {
        if (stepY != 0 && this->tryStep(0, stepY, world)) return;
        if (stepX != 0) this->tryStep(stepX, 0, world);
    }
}

std::string Zombie::getName() const
{
    return "Zombie";
}

/* -------------------------------------------------------------------------- */
/*  Bat                                                                       */
/* -------------------------------------------------------------------------- */

Bat::Bat(int x, int y)
    : Monster(x, y, 15, 'B', 5)
{
}

void Bat::takeTurn(IWorld& world)
{
    if (this->attackIfAdjacent(world))
    {
        return;
    }

    static constexpr int DX[4] = { 0, 0, 1, -1 };
    static constexpr int DY[4] = { -1, 1, 0, 0 };

    int start = std::rand() % 4;
    for (int i = 0; i < 4; ++i)
    {
        int d = (start + i) % 4;
        if (this->tryStep(DX[d], DY[d], world))
        {
            return;
        }
    }
}

std::string Bat::getName() const
{
    return "Bat";
}

/* -------------------------------------------------------------------------- */
/*  Guard                                                                     */
/* -------------------------------------------------------------------------- */

Guard::Guard(int x, int y)
    : Monster(x, y, 60, 'G', 15)
{
}

void Guard::takeTurn(IWorld& world)
{
    this->attackIfAdjacent(world);
}

std::string Guard::getName() const
{
    return "Guard";
}
