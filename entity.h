#ifndef ENTITY_H
#define ENTITY_H

#include <stddef.h>
#include "errors.h"

struct Map;

typedef struct Hero
{
    int health;
    int max_health;
    int attack;

    unsigned int gold;
    unsigned int floor;
    unsigned int states;

    /* Position: x - column, y - row */
    int x;
    int y;
} Hero;

typedef struct Monster
{
    int id;
    char name[32];
    char symbol;
    int hp;
    int max_hp;
    int attack;

    /* Position: x - column, y - row */
    int x;
    int y;
} Monster;

typedef struct MonsterList
{
    Monster* data;
    size_t count;
    size_t capacity;
} MonsterList;

/* Hero management */
void hero_init(Hero* hero, int start_x, int start_y);
int hero_damage(Hero* hero, int damage);
int hero_heal(Hero* hero, int amount);
int hero_add_gold(Hero* hero, unsigned int amount);

/* Monster list management (Practice 7 requirements 3, 4, 5, 6) */
ErrorCode monster_list_init(MonsterList* list, size_t initial_capacity);
ErrorCode monster_list_add(MonsterList* list, const Monster* monster);
int monster_list_remove(MonsterList* list, size_t index);
int monster_list_remove_at(MonsterList* list, int x, int y);
Monster* monster_list_get(MonsterList* list, size_t index);
Monster* monster_list_find_at(MonsterList* list, int x, int y);
int monster_list_find_index_at(const MonsterList* list, int x, int y);
void monster_list_free(MonsterList* list);

#endif /* ENTITY_H */
