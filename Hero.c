#include <stddef.h>
#include "Hero.h"

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
