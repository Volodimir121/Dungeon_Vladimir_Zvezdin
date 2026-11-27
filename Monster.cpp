#include "Monster.h"
#include "Map.h"

#include <array>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <utility>

namespace
{
constexpr std::array<Direction, 4> directions = {{{0, -1}, {0, 1}, {1, 0}, {-1, 0}}};

std::size_t randomIndex(std::size_t last)
{
    static std::mt19937 engine(std::random_device{}());
    return std::uniform_int_distribution<std::size_t>(0, last)(engine);
}

int sign(int value)
{
    return (value > 0) - (value < 0);
}

bool canStep(const Creature& creature, const IWorld& world, Direction step)
{
    if (step.dx < -1 || step.dx > 1 || step.dy < -1 || step.dy > 1 ||
        std::abs(step.dx) + std::abs(step.dy) != 1)
    {
        return false;
    }
    const int x = creature.getX() + step.dx;
    const int y = creature.getY() + step.dy;
    return world.getMap().canEnter(x, y) && !world.isCellOccupied(x, y);
}

Direction relativeStep(const Creature& creature, const IWorld& world, int sense)
{
    const Creature& player = world.getPlayer();
    const int dx = player.getX() - creature.getX();
    const int dy = player.getY() - creature.getY();
    Direction first{sign(dx) * sense, 0};
    Direction second{0, sign(dy) * sense};
    if (std::abs(dy) > std::abs(dx))
    {
        std::swap(first, second);
    }
    if (canStep(creature, world, first))
    {
        return first;
    }
    if (canStep(creature, world, second))
    {
        return second;
    }
    return {};
}
}

namespace MonsterRules
{
const MoveRule chase = [](const Creature& creature, const IWorld& world)
{
    return relativeStep(creature, world, 1);
};

const MoveRule flee = [](const Creature& creature, const IWorld& world)
{
    const Direction away = relativeStep(creature, world, -1);
    if (away.dx != 0 || away.dy != 0)
    {
        return away;
    }

    // При совпадении координат по одной оси шаг вдоль неё тоже увеличивает расстояние.
    const Creature& player = world.getPlayer();
    for (Direction step : directions)
    {
        if ((step.dx != 0 ? player.getX() == creature.getX() : player.getY() == creature.getY()) &&
            canStep(creature, world, step))
        {
            return step;
        }
    }
    return Direction{};
};

const MoveRule stay = [](const Creature&, const IWorld&)
{
    return Direction{};
};

const MoveRule wander = [](const Creature& creature, const IWorld& world)
{
    const std::size_t start = randomIndex(directions.size() - 1);
    for (std::size_t offset = 0; offset < directions.size(); ++offset)
    {
        const Direction step = directions[(start + offset) % directions.size()];
        if (canStep(creature, world, step))
        {
            return step;
        }
    }
    return Direction{};
};

MoveRule drunk(MoveRule baseRule)
{
    if (!baseRule)
    {
        throw std::invalid_argument("Исходное правило движения не может быть пустым.");
    }
    // Функция высшего порядка принимает правило и возвращает новое правило.
    // Замыкание владеет исходным правилом, поэтому его можно сохранить в монстре.
    return [baseRule = std::move(baseRule)](const Creature& creature, const IWorld& world)
    {
        if (randomIndex(5) == 0)
        {
            return directions[randomIndex(directions.size() - 1)];
        }
        return baseRule(creature, world);
    };
}
}

Monster::Monster(int x, int y, int maxHealth, char symbol, int attackDamage, MoveRule rule)
    : Creature(x, y, maxHealth, symbol), attackDamage(attackDamage)
{
    /* Счётчик растёт только после успешной проверки инварианта. */
    if (this->attackDamage < 0)
    {
        throw std::invalid_argument("Урон монстра не может быть отрицательным.");
    }
    this->setMoveRule(std::move(rule));
    ++Monster::totalCreated;
}

int Monster::getTotalCreated()
{
    return Monster::totalCreated;
}

void Monster::setMoveRule(MoveRule rule)
{
    if (!rule)
    {
        throw std::invalid_argument("Правило движения не может быть пустым.");
    }
    this->moveRule = std::move(rule);
}

int Monster::getAttackDamage() const
{
    return this->attackDamage;
}

bool Monster::canAttack(const Creature& player) const
{
    return this->isAlive() && player.isAlive() && this->distanceTo(player) == 1;
}

void Monster::takeTurn(IWorld& world)
{
    if (!this->isAlive())
    {
        return;
    }
    Creature& player = world.getPlayer();
    if (this->canAttack(player))
    {
        player.takeDamage(this->attackDamage);
        world.report(this->getName() + " hits you for " + std::to_string(this->attackDamage) + " damage.");
        return;
    }

    const Direction step = this->moveRule(*this, world);
    if (canStep(*this, world, step))
    {
        this->setPosition(this->getX() + step.dx, this->getY() + step.dy);
    }
}

Zombie::Zombie(int x, int y)
    : Monster(x, y, 40, 'Z', 10, [](const Creature& creature, const IWorld& world)
      {
          return creature.distanceTo(world.getPlayer()) <= VISION_RADIUS
              ? MonsterRules::chase(creature, world)
              : Direction{};
      })
{
}

std::string Zombie::getName() const
{
    return "Zombie";
}

Bat::Bat(int x, int y)
    : Monster(x, y, 15, 'B', 5, MonsterRules::wander)
{
}

std::string Bat::getName() const
{
    return "Bat";
}

Guard::Guard(int x, int y)
    : Monster(x, y, 60, 'G', 15, MonsterRules::stay)
{
}

std::string Guard::getName() const
{
    return "Guard";
}
