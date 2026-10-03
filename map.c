#include <stdio.h>

#include "map.h"


static char MAP[MAP_ROWS][MAP_COLS + 1] = {
    "####################",
    "#      #   $      E#",
    "#      #           #",
    "#      #      $    #",
    "#                  #",
    "#            $     #",
    "####################"
};


static int visited[MAP_ROWS][MAP_COLS];

int map_in_bounds(int x, int y)
{
    return x >= 0 && x < MAP_COLS && y >= 0 && y < MAP_ROWS;
}

int map_is_wall(int x, int y)
{
    return MAP[y][x] == MAP_WALL;
}

int map_can_enter(int x, int y)
{
    if (!map_in_bounds(x, y))
    {
        return 0;
    }

    return !map_is_wall(x, y);
}

char map_get(int x, int y)
{
    return MAP[y][x];
}

void map_clear(int x, int y)
{
    MAP[y][x] = MAP_FLOOR;
}

void map_print(int hero_x, int hero_y)
{
    for (int row = 0; row < MAP_ROWS; row++)
    {
        for (int col = 0; col < MAP_COLS; col++)
        {
            if (row == hero_y && col == hero_x)
            {
                putchar('@');
            }
            else
            {
                putchar(MAP[row][col]);
            }
        }

        putchar('\n');
    }
}


static int explore(int x, int y)
{
    if (!map_in_bounds(x, y))
    {
        return 0;
    }

    if (map_is_wall(x, y))
    {
        return 0;
    }

    if (visited[y][x])
    {
        return 0;
    }

    visited[y][x] = 1;

    if (map_get(x, y) == MAP_EXIT)
    {
        return 1;
    }

    if (explore(x + 1, y)) return 1;
    if (explore(x - 1, y)) return 1;
    if (explore(x, y + 1)) return 1;
    if (explore(x, y - 1)) return 1;

    return 0;
}

int map_check_reachable(int start_x, int start_y)
{
    for (int row = 0; row < MAP_ROWS; row++)
    {
        for (int col = 0; col < MAP_COLS; col++)
        {
            visited[row][col] = 0;
        }
    }

    return explore(start_x, start_y);
}