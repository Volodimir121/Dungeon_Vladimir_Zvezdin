#ifndef DUNGEON_VLADIMIR_ZVEZDIN_HERO_H
#define DUNGEON_VLADIMIR_ZVEZDIN_HERO_H

typedef struct
{
    int health;
    int max_health;
    int attack;

    unsigned int gold;
    unsigned int floor;
    unsigned int states;

    /* Позиция на карте: x — столбец, y — строка (см. map.c). */
    int x;
    int y;
} Hero;

void hero_init(Hero* hero);

int shift_position(int* x, int* y, int dx, int dy);

int damage_health(int* health, int damage);

int add_gold(unsigned int* gold, unsigned int amount);

#endif
