#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entity.h"

void hero_init(Hero* hero, int start_x, int start_y)
{
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

    hero->x = start_x;
    hero->y = start_y;
}

int hero_damage(Hero* hero, int damage)
{
    if (hero == NULL || damage < 0)
    {
        return 0;
    }

    hero->health -= damage;
    if (hero->health < 0)
    {
        hero->health = 0;
    }

    return 1;
}

int hero_heal(Hero* hero, int amount)
{
    if (hero == NULL || amount < 0)
    {
        return 0;
    }

    hero->health += amount;
    if (hero->health > hero->max_health)
    {
        hero->health = hero->max_health;
    }

    return 1;
}

int hero_add_gold(Hero* hero, unsigned int amount)
{
    if (hero == NULL)
    {
        return 0;
    }

    hero->gold += amount;
    return 1;
}

ErrorCode monster_list_init(MonsterList* list, size_t initial_capacity)
{
    if (list == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    list->count = 0;
    list->capacity = 0;
    list->data = NULL;

    if (initial_capacity > 0)
    {
        Monster* new_data = (Monster*)malloc(initial_capacity * sizeof(Monster));
        if (new_data == NULL)
        {
            return ERR_OUT_OF_MEMORY;
        }

        list->data = new_data;
        list->capacity = initial_capacity;
    }

    return ERR_OK;
}

ErrorCode monster_list_add(MonsterList* list, const Monster* monster)
{
    if (list == NULL || monster == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    if (list->count >= list->capacity)
    {
        /* Double capacity when full (initial 2) */
        size_t new_capacity = (list->capacity == 0) ? 2 : list->capacity * 2;
        Monster* new_data = (Monster*)realloc(list->data, new_capacity * sizeof(Monster));
        if (new_data == NULL)
        {
            /* Old pointer remains valid; do not overwrite list->data */
            return ERR_OUT_OF_MEMORY;
        }

        list->data = new_data;
        list->capacity = new_capacity;
    }

    list->data[list->count] = *monster;
    list->count++;

    return ERR_OK;
}

int monster_list_remove(MonsterList* list, size_t index)
{
    if (list == NULL || index >= list->count)
    {
        return 0;
    }

    for (size_t i = index; i + 1 < list->count; i++)
    {
        list->data[i] = list->data[i + 1];
    }

    list->count--;
    return 1;
}

int monster_list_remove_at(MonsterList* list, int x, int y)
{
    int index = monster_list_find_index_at(list, x, y);
    if (index < 0)
    {
        return 0;
    }

    return monster_list_remove(list, (size_t)index);
}

Monster* monster_list_get(MonsterList* list, size_t index)
{
    if (list == NULL || index >= list->count)
    {
        return NULL;
    }

    return &list->data[index];
}

Monster* monster_list_find_at(MonsterList* list, int x, int y)
{
    if (list == NULL || list->data == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < list->count; i++)
    {
        if (list->data[i].x == x && list->data[i].y == y && list->data[i].hp > 0)
        {
            return &list->data[i];
        }
    }

    return NULL;
}

int monster_list_find_index_at(const MonsterList* list, int x, int y)
{
    if (list == NULL || list->data == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < list->count; i++)
    {
        if (list->data[i].x == x && list->data[i].y == y && list->data[i].hp > 0)
        {
            return (int)i;
        }
    }

    return -1;
}

void monster_list_free(MonsterList* list)
{
    if (list == NULL)
    {
        return;
    }

    if (list->data != NULL)
    {
        free(list->data);
        list->data = NULL;
    }

    list->count = 0;
    list->capacity = 0;
}
