#ifndef MAP_H
#define MAP_H

#define MAP_ROWS 7
#define MAP_COLS 20

#define MAP_WALL  '#'
#define MAP_FLOOR ' '
#define MAP_GOLD  '$'
#define MAP_EXIT  'E'

int map_in_bounds(int x, int y);

int map_is_wall(int x, int y);

int map_can_enter(int x, int y);

char map_get(int x, int y);

void map_clear(int x, int y);

void map_print(int hero_x, int hero_y);

int map_check_reachable(int start_x, int start_y);

typedef enum
{
    CELL_FLOOR,
    CELL_WALL,
    CELL_GOLD,
    CELL_EXIT
} CellType;

typedef const char (*MapGrid)[MAP_COLS + 1];

MapGrid map_grid();

int map_cell_info(const char grid[][MAP_COLS + 1], int x, int y,
                  CellType* out_type, int* out_passable);

void map_print_addresses(void);

#endif