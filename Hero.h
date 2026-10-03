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

    int x;
    int y;
} Hero;

void hero_init(Hero* hero);
void hero_take_damage(Hero* hero, int damage);

#endif
