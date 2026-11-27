#pragma once
#include "Creature.h"

#include <functional>

struct Direction
{
    int dx = 0;
    int dy = 0;
};

using MoveRule = std::function<Direction(const Creature&, const IWorld&)>;

namespace MonsterRules
{
    extern const MoveRule chase;
    extern const MoveRule flee;
    extern const MoveRule stay;
    extern const MoveRule wander;
    MoveRule drunk(MoveRule baseRule);
}

/*
 * Monster — абстрактный базовый класс монстров, наследник Creature.
 * Наследование оправдано: монстр действительно является существом и может
 * использоваться везде, где ожидается Creature (подсказка практики 12).
 *
 * ИНВАРИАНТ КЛАССА (практика 11, критерий 1):
 *   все утверждения инварианта Creature, а также attackDamage >= 0.
 *   totalCreated равен числу успешно сконструированных монстров всех видов
 *   и только растёт.
 */
class Monster : public Creature
{
public:
    ~Monster() override = default;

    /* Сколько монстров было создано за всё время работы программы */
    static int getTotalCreated();

    void setMoveRule(MoveRule rule);
    void takeTurn(IWorld& world) override final;
    int getAttackDamage() const;
    bool canAttack(const Creature& player) const;

protected:
    Monster(int x, int y, int maxHealth, char symbol, int attackDamage, MoveRule rule);

private:
    /*
     * Практика 11, задание 6: счётчик созданных монстров СТАТИЧЕСКИЙ, потому что
     * это свойство класса Monster в целом, а не отдельного монстра.
     * Если бы поле было обычным, у каждого монстра была бы своя копия, равная 1 или 0,
     * и сосчитать «всех» было бы нельзя. Статический член существует в единственном
     * экземпляре, общем для всех объектов и всех наследников, создаётся до первого
     * монстра и читается без объекта: Monster::getTotalCreated().
     */
    inline static int totalCreated = 0;

    int attackDamage;
    MoveRule moveRule;
};

/* Зомби: преследует героя, если тот ближе VISION_RADIUS клеток, и бьёт вплотную */
class Zombie final : public Monster
{
public:
    static constexpr int VISION_RADIUS = 8;

    Zombie(int x, int y);
    std::string getName() const override;
};

/* Летучая мышь: блуждает в случайном направлении, слабо кусает, если рядом герой */
class Bat final : public Monster
{
public:
    Bat(int x, int y);
    std::string getName() const override;
};

/* Страж: стоит на месте и атакует только вплотную, но сильно */
class Guard final : public Monster
{
public:
    Guard(int x, int y);
    std::string getName() const override;
};
