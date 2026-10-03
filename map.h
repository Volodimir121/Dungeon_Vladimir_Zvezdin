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

#endif