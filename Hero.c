#include "hero.h"
 
void hero_init(Hero* hero)
{
    hero->health = 100;
    hero->max_health = 100;
    hero->attack = 10;
 
    hero->gold = 0;
    hero->floor = 1;
    hero->states = 0;
    
    hero->x = 2;
    hero->y = 1;
}
 
void hero_take_damage(Hero* hero, int damage)
{
    hero->health -= damage;
 
    if (hero->health < 0)
    {
        hero->health = 0;
    }
 
    if (hero->health > hero->max_health)
    {
        hero->health = hero->max_health;
    }
}