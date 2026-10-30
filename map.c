#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "map.h"

ErrorCode map_init(Map* map, int rows, int cols)
{
    if (map == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    if (rows < 3 || cols < 3)
    {
        return ERR_INVALID_SIZE;
    }

    map->rows = rows;
    map->cols = cols;
    map->data = (char*)malloc((size_t)rows * (size_t)cols * sizeof(char));

    if (map->data == NULL)
    {
        map->rows = 0;
        map->cols = 0;
        return ERR_OUT_OF_MEMORY;
    }

    memset(map->data, MAP_FLOOR, (size_t)rows * (size_t)cols * sizeof(char));
    return ERR_OK;
}

void map_free(Map* map)
{
    if (map == NULL)
    {
        return;
    }

    if (map->data != NULL)
    {
        free(map->data);
        map->data = NULL;
    }

    map->rows = 0;
    map->cols = 0;
}

int map_in_bounds(const Map* map, int x, int y)
{
    if (map == NULL || map->data == NULL)
    {
        return 0;
    }

    return (x >= 0 && x < map->cols && y >= 0 && y < map->rows);
}

int map_is_wall(const Map* map, int x, int y)
{
    return map_get(map, x, y) == MAP_WALL;
}

int map_can_enter(const Map* map, int x, int y)
{
    if (!map_in_bounds(map, x, y))
    {
        return 0;
    }

    return !map_is_wall(map, x, y);
}

char map_get_cell(const Map* map, int row, int col)
{
    if (map == NULL || map->data == NULL)
    {
        return MAP_WALL;
    }

    if (row < 0 || row >= map->rows || col < 0 || col >= map->cols)
    {
        return MAP_WALL;
    }

    return map->data[row * map->cols + col];
}

void map_set_cell(Map* map, int row, int col, char ch)
{
    if (map == NULL || map->data == NULL)
    {
        return;
    }

    if (row < 0 || row >= map->rows || col < 0 || col >= map->cols)
    {
        return;
    }

    map->data[row * map->cols + col] = ch;
}

char map_get(const Map* map, int x, int y)
{
    return map_get_cell(map, y, x);
}

void map_set(Map* map, int x, int y, char ch)
{
    map_set_cell(map, y, x, ch);
}

void map_clear(Map* map, int x, int y)
{
    map_set(map, x, y, MAP_FLOOR);
}

int map_cell_info(const Map* map, int x, int y, CellType* out_type, int* out_passable)
{
    if (map == NULL || out_type == NULL || out_passable == NULL)
    {
        return 0;
    }

    if (!map_in_bounds(map, x, y))
    {
        return 0;
    }

    char cell = map_get(map, x, y);

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

static int explore_reachable(const Map* map, int* visited, int x, int y)
{
    if (!map_in_bounds(map, x, y))
    {
        return 0;
    }

    if (map_is_wall(map, x, y))
    {
        return 0;
    }

    int idx = y * map->cols + x;
    if (visited[idx])
    {
        return 0;
    }

    visited[idx] = 1;

    if (map_get(map, x, y) == MAP_EXIT)
    {
        return 1;
    }

    if (explore_reachable(map, visited, x + 1, y)) return 1;
    if (explore_reachable(map, visited, x - 1, y)) return 1;
    if (explore_reachable(map, visited, x, y + 1)) return 1;
    if (explore_reachable(map, visited, x, y - 1)) return 1;

    return 0;
}

int map_check_reachable(const Map* map, int start_x, int start_y)
{
    if (map == NULL || map->data == NULL || map->rows <= 0 || map->cols <= 0)
    {
        return 0;
    }

    int* visited = (int*)calloc((size_t)map->rows * (size_t)map->cols, sizeof(int));
    if (visited == NULL)
    {
        return 0;
    }

    int reachable = explore_reachable(map, visited, start_x, start_y);

    free(visited);
    visited = NULL;

    return reachable;
}

static char* read_line_from_file(FILE* f, size_t* out_len)
{
    size_t cap = 64;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (buf == NULL)
    {
        return NULL;
    }

    int c;
    while ((c = fgetc(f)) != EOF)
    {
        if (c == '\r')
        {
            int next = fgetc(f);
            if (next != '\n' && next != EOF)
            {
                ungetc(next, f);
            }
            break;
        }
        if (c == '\n')
        {
            break;
        }

        if (len + 1 >= cap)
        {
            size_t new_cap = cap * 2;
            char* new_buf = (char*)realloc(buf, new_cap);
            if (new_buf == NULL)
            {
                free(buf);
                return NULL;
            }
            buf = new_buf;
            cap = new_cap;
        }

        buf[len++] = (char)c;
    }

    if (c == EOF && len == 0)
    {
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    if (out_len != NULL)
    {
        *out_len = len;
    }

    return buf;
}

ErrorCode map_load_from_file(const char* filename, Map* out_map,
                           int* out_hero_x, int* out_hero_y,
                           MonsterList* out_monsters)
{
    if (filename == NULL || out_map == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    FILE* f = fopen(filename, "r");
    if (f == NULL)
    {
        return ERR_FILE_NOT_FOUND;
    }

    ErrorCode err = ERR_OK;
    char* line = NULL;
    size_t cols = 0;
    int rows = 0;

    /* First pass: validate file structure, dimensions, and line consistency */
    line = read_line_from_file(f, &cols);
    if (line == NULL)
    {
        err = ERR_BAD_FORMAT; /* Empty file */
        goto cleanup;
    }

    if (cols < 3)
    {
        free(line);
        line = NULL;
        err = ERR_INVALID_SIZE;
        goto cleanup;
    }

    free(line);
    line = NULL;
    rows = 1;

    while (1)
    {
        size_t cur_len = 0;
        line = read_line_from_file(f, &cur_len);
        if (line == NULL)
        {
            break;
        }

        /* Allow optional empty line at the very end of file */
        if (cur_len == 0)
        {
            free(line);
            line = NULL;
            continue;
        }

        if (cur_len != cols)
        {
            free(line);
            line = NULL;
            err = ERR_BAD_FORMAT; /* Inconsistent line lengths */
            goto cleanup;
        }

        free(line);
        line = NULL;
        rows++;
    }

    if (rows < 3)
    {
        err = ERR_INVALID_SIZE;
        goto cleanup;
    }

    /* Allocate dynamic map */
    err = map_init(out_map, rows, (int)cols);
    if (err != ERR_OK)
    {
        goto cleanup;
    }

    /* Second pass: rewind file and fill cells */
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        err = ERR_BAD_FORMAT;
        goto cleanup;
    }

    int hero_found = 0;
    int monster_id_counter = 1;

    for (int r = 0; r < rows; r++)
    {
        size_t len = 0;
        line = read_line_from_file(f, &len);
        if (line == NULL || len != cols)
        {
            if (line != NULL)
            {
                free(line);
                line = NULL;
            }
            err = ERR_BAD_FORMAT;
            goto cleanup;
        }

        for (int c = 0; c < (int)cols; c++)
        {
            char ch = line[c];
            if (ch == MAP_HERO)
            {
                if (out_hero_x) *out_hero_x = c;
                if (out_hero_y) *out_hero_y = r;
                hero_found = 1;
                map_set_cell(out_map, r, c, MAP_FLOOR);
            }
            else if (ch == MAP_MONSTER)
            {
                if (out_monsters != NULL)
                {
                    Monster m;
                    m.id = monster_id_counter++;
                    snprintf(m.name, sizeof(m.name), "Goblin_%d", m.id);
                    m.symbol = MAP_MONSTER;
                    m.hp = 25;
                    m.max_hp = 25;
                    m.attack = 6;
                    m.x = c;
                    m.y = r;
                    err = monster_list_add(out_monsters, &m);
                    if (err != ERR_OK)
                    {
                        free(line);
                        line = NULL;
                        goto cleanup;
                    }
                }
                map_set_cell(out_map, r, c, MAP_FLOOR);
            }
            else if (ch == MAP_WALL || ch == MAP_FLOOR || ch == MAP_GOLD || ch == MAP_EXIT)
            {
                map_set_cell(out_map, r, c, ch);
            }
            else
            {
                /* Unrecognized character in level file */
                free(line);
                line = NULL;
                err = ERR_BAD_FORMAT;
                goto cleanup;
            }
        }

        free(line);
        line = NULL;
    }

    /* If no hero position specified, find first open floor */
    if (!hero_found)
    {
        int placed = 0;
        for (int r = 1; r < rows - 1 && !placed; r++)
        {
            for (int c = 1; c < (int)cols - 1 && !placed; c++)
            {
                if (map_get_cell(out_map, r, c) == MAP_FLOOR)
                {
                    if (out_hero_x) *out_hero_x = c;
                    if (out_hero_y) *out_hero_y = r;
                    placed = 1;
                }
            }
        }

        if (!placed)
        {
            if (out_hero_x) *out_hero_x = 1;
            if (out_hero_y) *out_hero_y = 1;
        }
    }

cleanup:
    if (line != NULL)
    {
        free(line);
        line = NULL;
    }

    if (f != NULL)
    {
        fclose(f);
        f = NULL;
    }

    if (err != ERR_OK)
    {
        map_free(out_map);
        if (out_monsters != NULL)
        {
            monster_list_free(out_monsters);
        }
    }

    return err;
}

ErrorCode map_generate(Map* map, int rows, int cols,
                       int* out_hero_x, int* out_hero_y,
                       MonsterList* out_monsters)
{
    if (map == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    if (rows < 4 || cols < 4)
    {
        return ERR_INVALID_SIZE;
    }

    ErrorCode err = map_init(map, rows, cols);
    if (err != ERR_OK)
    {
        return err;
    }

    static int seeded = 0;
    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    /* Target proportions: MAP_WALL_PERCENT (30%), MAP_GOLD_PERCENT (3%), MAP_FLOOR_PERCENT (67%) */
    int total_cells = rows * cols;
    int target_walls = (int)(total_cells * ((float)MAP_WALL_PERCENT / 100.0f) + 0.5f);
    int target_gold = (int)(total_cells * ((float)MAP_GOLD_PERCENT / 100.0f) + 0.5f);
    if (target_gold < 1)
    {
        target_gold = 1;
    }

    /* 1. Set border walls */
    int border_walls = 0;
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (r == 0 || r == rows - 1 || c == 0 || c == cols - 1)
            {
                map_set_cell(map, r, c, MAP_WALL);
                border_walls++;
            }
            else
            {
                map_set_cell(map, r, c, MAP_FLOOR);
            }
        }
    }

    /* 2. Place interior walls if needed */
    int inner_walls = target_walls > border_walls ? (target_walls - border_walls) : 0;
    int inner_cells = (rows - 2) * (cols - 2);

    /* Guard against overwhelming interior with walls */
    if (inner_walls > inner_cells / 3)
    {
        inner_walls = inner_cells / 3;
    }

    int walls_placed = 0;
    while (walls_placed < inner_walls)
    {
        int r = 1 + rand() % (rows - 2);
        int c = 1 + rand() % (cols - 2);
        if (map_get_cell(map, r, c) == MAP_FLOOR)
        {
            map_set_cell(map, r, c, MAP_WALL);
            walls_placed++;
        }
    }

    /* 3. Place gold */
    int gold_placed = 0;
    int attempts = 0;
    while (gold_placed < target_gold && attempts < inner_cells * 4)
    {
        attempts++;
        int r = 1 + rand() % (rows - 2);
        int c = 1 + rand() % (cols - 2);
        if (map_get_cell(map, r, c) == MAP_FLOOR)
        {
            map_set_cell(map, r, c, MAP_GOLD);
            gold_placed++;
        }
    }

    /* 4. Place hero */
    int hx = 1, hy = 1;
    attempts = 0;
    while (attempts < 500)
    {
        int r = 1 + rand() % (rows - 2);
        int c = 1 + rand() % (cols - 2);
        if (map_get_cell(map, r, c) == MAP_FLOOR)
        {
            hx = c;
            hy = r;
            break;
        }
        attempts++;
    }
    if (out_hero_x) *out_hero_x = hx;
    if (out_hero_y) *out_hero_y = hy;

    /* 5. Place exit */
    int ex = cols - 2, ey = rows - 2;
    map_set_cell(map, ey, ex, MAP_EXIT);

    /* 6. Verify and ensure exit reachability */
    if (!map_check_reachable(map, hx, hy))
    {
        /* Carve a path from hero to exit */
        int cur_x = hx;
        int cur_y = hy;

        while (cur_x != ex)
        {
            cur_x += (ex > cur_x) ? 1 : -1;
            if (map_get_cell(map, cur_y, cur_x) == MAP_WALL)
            {
                map_set_cell(map, cur_y, cur_x, MAP_FLOOR);
            }
        }

        while (cur_y != ey)
        {
            cur_y += (ey > cur_y) ? 1 : -1;
            if (map_get_cell(map, cur_y, cur_x) == MAP_WALL)
            {
                map_set_cell(map, cur_y, cur_x, MAP_FLOOR);
            }
        }

        map_set_cell(map, ey, ex, MAP_EXIT);
    }

    /* 7. Spawn monsters in MonsterList */
    if (out_monsters != NULL)
    {
        int monster_count = (rows * cols) / 40 + 1;
        if (monster_count < 1) monster_count = 1;
        if (monster_count > 8) monster_count = 8;

        for (int i = 0; i < monster_count; i++)
        {
            int mx = 0, my = 0;
            attempts = 0;
            while (attempts < 200)
            {
                int r = 1 + rand() % (rows - 2);
                int c = 1 + rand() % (cols - 2);
                if (map_get_cell(map, r, c) == MAP_FLOOR && !(c == hx && r == hy))
                {
                    mx = c;
                    my = r;
                    break;
                }
                attempts++;
            }

            if (attempts < 200)
            {
                Monster m;
                m.id = i + 1;
                snprintf(m.name, sizeof(m.name), (i % 2 == 0) ? "Goblin_%d" : "Orc_%d", m.id);
                m.symbol = MAP_MONSTER;
                m.hp = 20 + i * 5;
                m.max_hp = m.hp;
                m.attack = 5 + i * 2;
                m.x = mx;
                m.y = my;

                err = monster_list_add(out_monsters, &m);
                if (err != ERR_OK)
                {
                    map_free(map);
                    return err;
                }
            }
        }
    }

    return ERR_OK;
}

void map_print(const Map* map, const Hero* hero, const MonsterList* monsters)
{
    if (map == NULL || map->data == NULL)
    {
        return;
    }

    for (int r = 0; r < map->rows; r++)
    {
        for (int c = 0; c < map->cols; c++)
        {
            if (hero != NULL && c == hero->x && r == hero->y)
            {
                putchar(MAP_HERO);
            }
            else
            {
                int printed_monster = 0;
                if (monsters != NULL)
                {
                    for (size_t m = 0; m < monsters->count; m++)
                    {
                        if (monsters->data[m].x == c && monsters->data[m].y == r && monsters->data[m].hp > 0)
                        {
                            putchar(monsters->data[m].symbol);
                            printed_monster = 1;
                            break;
                        }
                    }
                }

                if (!printed_monster)
                {
                    putchar(map_get_cell(map, r, c));
                }
            }
        }
        putchar('\n');
    }
}

void map_print_addresses(const Map* map)
{
    if (map == NULL || map->data == NULL)
    {
        printf("Map is not allocated.\n");
        return;
    }

    printf("-- map: dynamic memory info --\n");
    printf("Map struct address (caller): %p\n", (void*)map);
    printf("Map data (heap block)      : %p\n", (void*)map->data);
    printf("Dimensions                 : %d rows x %d cols (%d total cells)\n",
           map->rows, map->cols, map->rows * map->cols);
    printf("Bytes allocated on heap    : %zu bytes\n",
           (size_t)map->rows * (size_t)map->cols * sizeof(char));
    printf("First cell &data[0]        : %p\n", (void*)&map->data[0]);
    printf("Last cell &data[N-1]       : %p\n",
           (void*)&map->data[map->rows * map->cols - 1]);
}
