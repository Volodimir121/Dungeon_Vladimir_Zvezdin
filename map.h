#ifndef MAP_H
#define MAP_H

#include <stddef.h>
#include "errors.h"
#include "entity.h"

#define MAP_WALL    '#'
#define MAP_FLOOR   ' '
#define MAP_GOLD    '$'
#define MAP_EXIT    'E'
#define MAP_HERO    '@'
#define MAP_MONSTER 'M'

/* Default generation proportions */
#define MAP_WALL_PERCENT    30
#define MAP_GOLD_PERCENT    3
#define MAP_FLOOR_PERCENT   (100 - MAP_WALL_PERCENT - MAP_GOLD_PERCENT)

typedef enum
{
    CELL_FLOOR,
    CELL_WALL,
    CELL_GOLD,
    CELL_EXIT
} CellType;

typedef struct Map
{
    int rows;
    int cols;
    char* data; /* 1D contiguous block: row * cols + col */
} Map;

/* Initialization and memory management */
ErrorCode map_init(Map* map, int rows, int cols);
void map_free(Map* map);

/* Coordinate and boundary checks */
int map_in_bounds(const Map* map, int x, int y);
int map_is_wall(const Map* map, int x, int y);
int map_can_enter(const Map* map, int x, int y);

/* Cell access by row and col (Practice 7 Requirement 2) */
char map_get_cell(const Map* map, int row, int col);
void map_set_cell(Map* map, int row, int col, char ch);

/* Cell access by x (col) and y (row) */
char map_get(const Map* map, int x, int y);
void map_set(Map* map, int x, int y, char ch);
void map_clear(Map* map, int x, int y);

int map_cell_info(const Map* map, int x, int y, CellType* out_type, int* out_passable);

/* Reachability check using dynamically allocated memory */
int map_check_reachable(const Map* map, int start_x, int start_y);

/* Loading level from file (Practice 8 Requirement 3 & 4) */
ErrorCode map_load_from_file(const char* filename, Map* out_map,
                           int* out_hero_x, int* out_hero_y,
                           MonsterList* out_monsters);

/* Procedural generation: 30% wall, 60% floor, 10% gold (Practice 7) */
ErrorCode map_generate(Map* map, int rows, int cols,
                       int* out_hero_x, int* out_hero_y,
                       MonsterList* out_monsters);

/* Rendering */
void map_print(const Map* map, const Hero* hero, const MonsterList* monsters);

/* Memory debug output */
void map_print_addresses(const Map* map);

#endif /* MAP_H */
