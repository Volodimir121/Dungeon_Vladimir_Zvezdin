#include <stddef.h>
#include <stdio.h>

#include "map.h"


static char MAP[MAP_ROWS][MAP_COLS + 1] = {
    "####################",
    "#      #   $      E#",
    "#      #           #",
    "#      #      $    #",
    "#  $               #",
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

MapGrid map_grid()
{
    return (MapGrid)MAP;
}

int map_cell_info(const char grid[][MAP_COLS + 1], int x, int y,
                  CellType* out_type, int* out_passable)
{
    if (grid == NULL || out_type == NULL || out_passable == NULL)
    {
        return 0;
    }

    if (!map_in_bounds(x, y))
    {
        return 0;
    }

    char cell = grid[y][x];

    switch (cell)
    {
    case MAP_WALL:
        *out_type = CELL_WALL;
        break;
    case MAP_GOLD:
        *out_type = CELL_GOLD;
        break;
    case MAP_EXIT:
        *out_type = CELL_EXIT;
        break;
    default:
        *out_type = CELL_FLOOR;
        break;
    }

    *out_passable = (*out_type != CELL_WALL);

    return 1;
}

static size_t grid_param_size(const char (*grid)[MAP_COLS + 1])
{
    return sizeof(grid);
}

void map_print_addresses(void)
{
    printf("-- map: addresses and sizes --\n");
    printf("MAP (static, initialized)  : %p, sizeof = %zu\n",
        (void*)MAP, sizeof(MAP));
    printf("&MAP[0][0]                 : %p\n", (void*)&MAP[0][0]);
    printf("&MAP[0][1]                 : %p (+%td byte)\n",
        (void*)&MAP[0][1], &MAP[0][1] - &MAP[0][0]);
    printf("&MAP[1][0]                 : %p (+%td bytes = one row)\n",
        (void*)&MAP[1][0], &MAP[1][0] - &MAP[0][0]);
    printf("&MAP[%d][%d]                : %p (last byte of the array)\n",
        MAP_ROWS - 1, MAP_COLS, (void*)&MAP[MAP_ROWS - 1][MAP_COLS]);
    printf("visited (static, zeroed)   : %p, sizeof = %zu\n",
        (void*)visited, sizeof(visited));
    printf("sizeof(grid) inside function: %zu (pointer, not array)\n",
        grid_param_size(map_grid()));
}