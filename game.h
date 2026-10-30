#ifndef GAME_H
#define GAME_H

#include "errors.h"
#include "map.h"
#include "entity.h"

typedef struct Game
{
    Map map;
    Hero hero;
    MonsterList monsters;
    int turn;
    int is_running;
} Game;

/* Lifecycle */
void game_init_empty(Game* game);
ErrorCode game_init_procedural(Game* game, int rows, int cols);
ErrorCode game_init_from_file(Game* game, const char* filename);
void game_free(Game* game);

/* Game loop */
void game_run(Game* game);

/* Entry point */
void run_game(void);

#endif /* GAME_H */
