#include <stddef.h>

#include "Hero.h"
#include "map.h"

void hero_init(Hero* hero)
{
    /* Разыменование ниже без проверки на NULL нельзя — защищаемся. */
    if (hero == NULL)
    {
        return;
    }

    hero->health = 100;
    hero->max_health = 100;
    hero->attack = 10;

    hero->gold = 0;
    hero->floor = 1;
    hero->states = 0;

    hero->x = 2;
    hero->y = 1;
}

int shift_position(int* x, int* y, int dx, int dy)
{
    if (x == NULL || y == NULL)
    {
        return 0;
    }

    int new_x = *x + dx;
    int new_y = *y + dy;

    if (!map_can_enter(new_x, new_y))
    {
        return 0;
    }

    *x = new_x;
    *y = new_y;

    return 1;
}

int damage_health(int* health, int damage)
{
    if (health == NULL || damage < 0)
    {
        return 0;
    }

    *health -= damage;

    if (*health < 0)
    {
        *health = 0;
    }

    return 1;
}

int add_gold(unsigned int* gold, unsigned int amount)
{
    if (gold == NULL)
    {
        return 0;
    }
    
    *gold += amount;

    return 1;
}