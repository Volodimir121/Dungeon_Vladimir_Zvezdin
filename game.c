#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "entity.h"
#include "map.h"
#include "parcer.h"
#include "statushero.h"

typedef int (*CommandFn)(Game* game, const char* arg);

typedef struct
{
    const char* name;
    const char* alias;
    const char* help;
    int max_args;
    int is_move;
    CommandFn fn;
} Command;

void game_init_empty(Game* game)
{
    if (game == NULL)
    {
        return;
    }

    memset(&game->map, 0, sizeof(Map));
    memset(&game->hero, 0, sizeof(Hero));
    memset(&game->monsters, 0, sizeof(MonsterList));
    game->turn = 1;
    game->is_running = 0;
}

ErrorCode game_init_procedural(Game* game, int rows, int cols)
{
    if (game == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    game_init_empty(game);

    ErrorCode err = monster_list_init(&game->monsters, 2);
    if (err != ERR_OK)
    {
        return err;
    }

    int hx = 1, hy = 1;
    err = map_generate(&game->map, rows, cols, &hx, &hy, &game->monsters);
    if (err != ERR_OK)
    {
        monster_list_free(&game->monsters);
        return err;
    }

    hero_init(&game->hero, hx, hy);
    game->turn = 1;
    game->is_running = 1;

    return ERR_OK;
}

ErrorCode game_init_from_file(Game* game, const char* filename)
{
    if (game == NULL || filename == NULL)
    {
        return ERR_BAD_FORMAT;
    }

    game_init_empty(game);

    ErrorCode err = monster_list_init(&game->monsters, 2);
    if (err != ERR_OK)
    {
        return err;
    }

    int hx = 1, hy = 1;
    err = map_load_from_file(filename, &game->map, &hx, &hy, &game->monsters);
    if (err != ERR_OK)
    {
        monster_list_free(&game->monsters);
        return err;
    }

    hero_init(&game->hero, hx, hy);
    game->turn = 1;
    game->is_running = 1;

    return ERR_OK;
}

void game_free(Game* game)
{
    if (game == NULL)
    {
        return;
    }

    map_free(&game->map);
    monster_list_free(&game->monsters);
    game->is_running = 0;
}

static void pick_up_gold(Game* game)
{
    if (game == NULL)
    {
        return;
    }

    CellType type;
    int passable;

    if (!map_cell_info(&game->map, game->hero.x, game->hero.y, &type, &passable))
    {
        return;
    }

    if (type == CELL_GOLD)
    {
        hero_add_gold(&game->hero, 1);
        map_clear(&game->map, game->hero.x, game->hero.y);
        printf("You found gold! Total gold: %u\n", game->hero.gold);
    }
    else if (type == CELL_EXIT)
    {
        printf("Congratulations! You reached the exit!\n");
        printf("Total Gold: %u | Turns: %d\n", game->hero.gold, game->turn);
        game->is_running = 0;
    }
}

static int move_hero(Game* game, int dx, int dy, const char* direction)
{
    if (game == NULL || !game->is_running)
    {
        return 0;
    }

    int nx = game->hero.x + dx;
    int ny = game->hero.y + dy;

    if (!map_in_bounds(&game->map, nx, ny))
    {
        printf("You can't go %s - boundary blocks the way.\n", direction);
        return 0;
    }

    if (map_is_wall(&game->map, nx, ny))
    {
        printf("You can't go %s - a wall blocks the way.\n", direction);
        return 0;
    }

    /* Check if a monster occupies the destination */
    Monster* monster = monster_list_find_at(&game->monsters, nx, ny);
    if (monster != NULL)
    {
        printf("You attack %s for %d damage!\n", monster->name, game->hero.attack);
        monster->hp -= game->hero.attack;

        if (monster->hp <= 0)
        {
            printf("You defeated %s!\n", monster->name);
            monster_list_remove_at(&game->monsters, nx, ny);
        }
        else
        {
            printf("%s HP: %d/%d.\n", monster->name, monster->hp, monster->max_hp);
            hero_damage(&game->hero, monster->attack);
            printf("%s strikes back for %d damage! (Your HP: %d/%d)\n",
                   monster->name, monster->attack, game->hero.health, game->hero.max_health);

            if (game->hero.health <= 0)
            {
                printf("You have died! Game over.\n");
                game->is_running = 0;
            }
        }
        return 0;
    }

    game->hero.x = nx;
    game->hero.y = ny;
    printf("You went %s.\n", direction);

    pick_up_gold(game);

    return 0;
}

static int cmd_north(Game* game, const char* arg)
{
    (void)arg;
    return move_hero(game, 0, -1, "north");
}

static int cmd_south(Game* game, const char* arg)
{
    (void)arg;
    return move_hero(game, 0, 1, "south");
}

static int cmd_east(Game* game, const char* arg)
{
    (void)arg;
    return move_hero(game, 1, 0, "east");
}

static int cmd_west(Game* game, const char* arg)
{
    (void)arg;
    return move_hero(game, -1, 0, "west");
}

static int cmd_look(Game* game, const char* arg)
{
    (void)arg;
    printf("You look around the dungeon.\n");
    map_print(&game->map, &game->hero, &game->monsters);
    return 0;
}

static int cmd_status(Game* game, const char* arg)
{
    (void)arg;
    float health_percent =
        ((float)game->hero.health / (float)game->hero.max_health) * 100.0f;

    printf("Health: %d/%d (%.1f%%)\n",
        game->hero.health, game->hero.max_health, health_percent);
    printf("Attack: %d\n", game->hero.attack);
    printf("Gold: %u\n", game->hero.gold);
    printf("Floor: %u\n", game->hero.floor);

    print_statuses(game->hero.states);
    return 0;
}

static int read_number_arg(const char* arg, const char* what,
    unsigned int max, unsigned int* out)
{
    if (arg == NULL)
    {
        printf("Missing argument: expected %s (a number).\n", what);
        return 0;
    }

    switch (parse_uint(arg, max, out))
    {
    case PARSE_OK:
        return 1;
    case PARSE_BAD_CHAR:
        printf("'%s' is not a valid number for %s: only digits 0-9 are allowed.\n",
            arg, what);
        break;
    case PARSE_TOO_BIG:
        printf("'%s' is too large for %s (maximum is %u).\n",
            arg, what, max);
        break;
    default:
        printf("Empty %s.\n", what);
        break;
    }

    return 0;
}

static int cmd_hit(Game* game, const char* arg)
{
    unsigned int damage;

    if (!read_number_arg(arg, "damage", INT_MAX, &damage))
    {
        return 0;
    }

    if (!hero_damage(&game->hero, (int)damage))
    {
        printf("Invalid damage value.\n");
        return 0;
    }

    printf("You took %u damage.\n", damage);
    printf("Health: %d/%d\n", game->hero.health, game->hero.max_health);

    if (game->hero.health <= 0)
    {
        printf("You have died! Game over.\n");
        game->is_running = 0;
    }

    return 0;
}

static int cmd_use(Game* game, const char* arg)
{
    (void)game;
    unsigned int item;

    if (!read_number_arg(arg, "item number", UINT_MAX, &item))
    {
        return 0;
    }

    printf("You use item %u.\n", item);
    return 0;
}

static const char* cell_type_name(CellType type)
{
    switch (type)
    {
    case CELL_WALL: return "wall";
    case CELL_GOLD: return "gold";
    case CELL_EXIT: return "exit";
    default:        return "floor";
    }
}

static int cmd_inspect(Game* game, const char* arg)
{
    (void)arg;
    static const struct
    {
        const char* name;
        int dx;
        int dy;
    } DIRECTIONS[] = {
        { "north", 0, -1 },
        { "south", 0,  1 },
        { "east",  1,  0 },
        { "west", -1,  0 },
    };

    for (size_t i = 0; i < sizeof(DIRECTIONS) / sizeof(DIRECTIONS[0]); i++)
    {
        int tx = game->hero.x + DIRECTIONS[i].dx;
        int ty = game->hero.y + DIRECTIONS[i].dy;

        CellType type;
        int passable;

        if (map_cell_info(&game->map, tx, ty, &type, &passable))
        {
            Monster* m = monster_list_find_at(&game->monsters, tx, ty);
            if (m != NULL)
            {
                printf("%-6s: %s (HP %d/%d, ATK %d)\n", DIRECTIONS[i].name,
                       m->name, m->hp, m->max_hp, m->attack);
            }
            else
            {
                printf("%-6s: %s, %s\n", DIRECTIONS[i].name,
                    cell_type_name(type), passable ? "passable" : "blocked");
            }
        }
        else
        {
            printf("%-6s: outside the map\n", DIRECTIONS[i].name);
        }
    }

    return 0;
}

static int cmd_addr(Game* game, const char* arg)
{
    (void)arg;
    int local = 0;

    printf("-- Memory Addresses & Layout --\n");
    printf("Stack:\n");
    printf("  local variable in cmd_addr: %p\n", (void*)&local);
    printf("  game struct (stack)       : %p\n", (void*)game);
    printf("  &game->hero               : %p\n", (void*)&game->hero);
    printf("  &game->map                : %p\n", (void*)&game->map);
    printf("  &game->monsters           : %p\n", (void*)&game->monsters);

    printf("\nHeap (Dynamic memory - Practice 7):\n");
    map_print_addresses(&game->map);

    printf("\nMonsters dynamic array:\n");
    printf("  Monsters data pointer     : %p\n", (void*)game->monsters.data);
    printf("  Count / Capacity          : %zu / %zu\n",
           game->monsters.count, game->monsters.capacity);
    printf("  Bytes allocated           : %zu bytes\n",
           game->monsters.capacity * sizeof(Monster));

    return 0;
}

static int cmd_check(Game* game, const char* arg)
{
    (void)arg;
    if (map_check_reachable(&game->map, game->hero.x, game->hero.y))
    {
        printf("The exit is reachable from here.\n");
    }
    else
    {
        printf("The exit is NOT reachable from here.\n");
    }

    return 0;
}

static int cmd_monsters(Game* game, const char* arg)
{
    (void)arg;
    printf("Monsters dynamic array: %zu count, %zu capacity\n",
           game->monsters.count, game->monsters.capacity);

    if (game->monsters.count == 0)
    {
        printf("No monsters present.\n");
        return 0;
    }

    for (size_t i = 0; i < game->monsters.count; i++)
    {
        Monster* m = &game->monsters.data[i];
        printf("  [%zu] %-12s at (%2d, %2d) | HP: %d/%d | ATK: %d\n",
               i + 1, m->name, m->x, m->y, m->hp, m->max_hp, m->attack);
    }

    return 0;
}

static int cmd_spawn(Game* game, const char* arg)
{
    /* Find a random floor cell */
    int placed = 0;
    int rx = 1, ry = 1;

    for (int attempts = 0; attempts < 500 && !placed; attempts++)
    {
        int r = 1 + rand() % (game->map.rows - 2);
        int c = 1 + rand() % (game->map.cols - 2);

        if (map_get(&game->map, c, r) == MAP_FLOOR &&
            !(c == game->hero.x && r == game->hero.y) &&
            monster_list_find_at(&game->monsters, c, r) == NULL)
        {
            rx = c;
            ry = r;
            placed = 1;
        }
    }

    if (!placed)
    {
        printf("Could not find an empty cell to spawn a monster.\n");
        return 0;
    }

    Monster m;
    m.id = (int)game->monsters.count + 1;
    if (arg != NULL && strlen(arg) > 0)
    {
        snprintf(m.name, sizeof(m.name), "%s", arg);
    }
    else
    {
        snprintf(m.name, sizeof(m.name), "Goblin_%d", m.id);
    }
    m.symbol = MAP_MONSTER;
    m.hp = 20;
    m.max_hp = 20;
    m.attack = 5;
    m.x = rx;
    m.y = ry;

    size_t old_cap = game->monsters.capacity;
    ErrorCode err = monster_list_add(&game->monsters, &m);

    if (err != ERR_OK)
    {
        printf("Failed to spawn monster: %s\n", error_code_to_string(err));
        return 0;
    }

    if (game->monsters.capacity > old_cap)
    {
        printf("[Dynamic array]: Capacity doubled from %zu to %zu!\n",
               old_cap, game->monsters.capacity);
    }

    printf("Spawned %s at (%d, %d). Total monsters: %zu (Capacity: %zu)\n",
           m.name, m.x, m.y, game->monsters.count, game->monsters.capacity);

    return 0;
}

static int cmd_kill(Game* game, const char* arg)
{
    if (arg == NULL)
    {
        printf("Usage: kill <monster_index> (see 'monsters' list)\n");
        return 0;
    }

    unsigned int idx = 0;
    if (!read_number_arg(arg, "monster index", (unsigned int)game->monsters.count, &idx))
    {
        return 0;
    }

    if (idx == 0 || idx > game->monsters.count)
    {
        printf("Invalid monster index %u. Current count is %zu.\n", idx, game->monsters.count);
        return 0;
    }

    char removed_name[32];
    snprintf(removed_name, sizeof(removed_name), "%s", game->monsters.data[idx - 1].name);

    if (monster_list_remove(&game->monsters, (size_t)(idx - 1)))
    {
        printf("Monster '%s' [#%u] removed. Remaining monsters: %zu\n",
               removed_name, idx, game->monsters.count);
    }

    return 0;
}

static int cmd_mapinfo(Game* game, const char* arg)
{
    (void)arg;
    if (game->map.data == NULL)
    {
        printf("Map not loaded.\n");
        return 0;
    }

    int walls = 0;
    int floor = 0;
    int gold = 0;
    int exits = 0;
    int total = game->map.rows * game->map.cols;

    for (int r = 0; r < game->map.rows; r++)
    {
        for (int c = 0; c < game->map.cols; c++)
        {
            char cell = map_get_cell(&game->map, r, c);
            switch (cell)
            {
            case MAP_WALL: walls++; break;
            case MAP_GOLD: gold++;  break;
            case MAP_EXIT: exits++; break;
            default:       floor++; break;
            }
        }
    }

    printf("-- Map Statistics --\n");
    printf("Dimensions : %d rows x %d cols (%d total cells)\n",
           game->map.rows, game->map.cols, total);
    printf("Walls (#)  : %3d (%.1f%%)\n", walls, (100.0f * walls) / (float)total);
    printf("Floor ( )  : %3d (%.1f%%)\n", floor, (100.0f * floor) / (float)total);
    printf("Gold  ($)  : %3d (%.1f%%)\n", gold,  (100.0f * gold)  / (float)total);
    printf("Exit  (E)  : %3d\n", exits);
    printf("Monsters   : %zu active\n", game->monsters.count);

    return 0;
}

static int cmd_quit(Game* game, const char* arg)
{
    (void)arg;
    game->is_running = 0;
    return 1;
}

static int cmd_help(Game* game, const char* arg);

static const Command COMMANDS[] = {
    { "north",    "n",   "go north",                      0, 1, cmd_north    },
    { "south",    "s",   "go south",                      0, 1, cmd_south    },
    { "east",     "e",   "go east",                       0, 1, cmd_east     },
    { "west",     "w",   "go west",                       0, 1, cmd_west     },
    { "look",     "l",   "look around the dungeon",       0, 0, cmd_look     },
    { "status",   "st",  "show hero status",              0, 0, cmd_status   },
    { "hit",      "",    "take damage: hit N",            1, 0, cmd_hit      },
    { "use",      "u",   "use an item: use N",            1, 0, cmd_use      },
    { "check",    "c",   "check if exit is reachable",    0, 0, cmd_check    },
    { "inspect",  "i",   "describe neighbouring cells",   0, 0, cmd_inspect  },
    { "monsters", "m",   "list all active monsters",      0, 0, cmd_monsters },
    { "spawn",    "addm","spawn monster: spawn [name]",   1, 0, cmd_spawn    },
    { "kill",     "delm","remove monster: kill <idx>",    1, 0, cmd_kill     },
    { "mapinfo",  "mi",  "display map stats and % ratios",0, 0, cmd_mapinfo  },
    { "addr",     "",    "print memory addresses",        0, 0, cmd_addr     },
    { "help",     "h",   "show this help",                0, 0, cmd_help     },
    { "quit",     "q",   "exit the game",                 0, 0, cmd_quit     },
};

static const size_t COMMAND_COUNT =
    sizeof(COMMANDS) / sizeof(COMMANDS[0]);

static int cmd_help(Game* game, const char* arg)
{
    (void)game;
    (void)arg;
    printf("Available commands (name, short form):\n");

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        printf("%-10s %-4s - %s\n",
            COMMANDS[i].name, COMMANDS[i].alias, COMMANDS[i].help);
    }

    printf("go <dir>        - same as north/south/east/west, e.g. go north\n");

    return 0;
}

static const Command* find_command(const char* word)
{
    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(word, COMMANDS[i].name) == 0)
        {
            return &COMMANDS[i];
        }

        if (COMMANDS[i].alias[0] != '\0' &&
            strcmp(word, COMMANDS[i].alias) == 0)
        {
            return &COMMANDS[i];
        }
    }

    return NULL;
}

static int process_command(Game* game, int argc, char* argv[])
{
    int first = 0;
    int only_moves = 0;

    if (strcmp(argv[0], "go") == 0)
    {
        if (argc < 2)
        {
            printf("Go where? Example: go north\n");
            return 0;
        }

        first = 1;
        only_moves = 1;
    }

    const Command* cmd = find_command(argv[first]);

    if (only_moves && (cmd == NULL || !cmd->is_move))
    {
        printf("You can't go '%s'.\n", argv[first]);
        return 0;
    }

    if (cmd == NULL)
    {
        if (process_status_command(argv[0], &game->hero.states))
        {
            return 0;
        }

        printf("Unknown command: '%s'. Type help.\n", argv[0]);
        return 0;
    }

    int arg_count = argc - first - 1;

    if (arg_count > cmd->max_args)
    {
        printf("'%s' takes %s.\n", cmd->name,
            cmd->max_args == 0 ? "no arguments" : "one argument");
        return 0;
    }

    return cmd->fn(game, arg_count > 0 ? argv[first + 1] : NULL);
}

void game_run(Game* game)
{
    if (game == NULL || !game->is_running)
    {
        return;
    }

    char line[LINE_BUFFER_SIZE];
    char* words[MAX_WORDS];

    printf("\nWelcome to the Dungeon!\n");
    map_print(&game->map, &game->hero, &game->monsters);
    printf("\nSymbols: @ - Player, M - Monster, $ - Gold, E - Exit, # - Wall\n");
    printf("Type 'help' for available commands.\n\n");

    while (game->is_running)
    {
        printf("Turn %d\n", game->turn);
        printf("> ");

        LineStatus status = read_line(line, sizeof(line));

        if (status == LINE_EOF || status == LINE_ERROR)
        {
            printf("\n");
            break;
        }

        if (status == LINE_TOO_LONG)
        {
            printf("Line is too long (max %d characters).\n\n",
                LINE_BUFFER_SIZE - 2);
            continue;
        }

        to_lower_ascii(line);

        int count = split_words(line, words, MAX_WORDS);

        if (count < 0)
        {
            printf("Too many words (max %d).\n\n", MAX_WORDS);
            continue;
        }

        if (count == 0)
        {
            continue;
        }

        if (process_command(game, count, words))
        {
            break;
        }

        if (game->is_running)
        {
            game->turn++;
            printf("\n");
        }
    }
}

void run_game(void)
{
    Game game;
    game_init_empty(&game);

    char line[LINE_BUFFER_SIZE];
    printf("=== Dungeon Crawler ===\n");
    printf("Select map mode:\n");
    printf("  1. Generate map by size (%d%% walls, %d%% floor, %d%% gold)\n",
           MAP_WALL_PERCENT, MAP_FLOOR_PERCENT, MAP_GOLD_PERCENT);
    printf("  2. Load level from file (Practice 8)\n");

    while (1)
    {
        printf("Choice (1 or 2) [1]: ");
        LineStatus st = read_line(line, sizeof(line));
        if (st == LINE_EOF || st == LINE_ERROR)
        {
            return;
        }

        if (line[0] == '\0' || strcmp(line, "1") == 0)
        {
            /* Mode 1: Procedural generation with requested ratios */
            int rows = 10;
            int cols = 20;

            printf("Enter rows [default 10]: ");
            if (read_line(line, sizeof(line)) == LINE_OK && line[0] != '\0')
            {
                unsigned int val;
                if (parse_uint(line, 100, &val) == PARSE_OK && val >= 3)
                {
                    rows = (int)val;
                }
                else
                {
                    printf("Invalid rows, using default 10.\n");
                }
            }

            printf("Enter cols [default 20]: ");
            if (read_line(line, sizeof(line)) == LINE_OK && line[0] != '\0')
            {
                unsigned int val;
                if (parse_uint(line, 100, &val) == PARSE_OK && val >= 3)
                {
                    cols = (int)val;
                }
                else
                {
                    printf("Invalid cols, using default 20.\n");
                }
            }

            ErrorCode err = game_init_procedural(&game, rows, cols);
            if (err != ERR_OK)
            {
                printf("Initialization error: %s\n", error_code_to_string(err));
                return;
            }
            break;
        }
        else if (strcmp(line, "2") == 0)
        {
            /* Mode 2: Load level from file */
            printf("Enter level file path [default level.txt]: ");
            char filename[LINE_BUFFER_SIZE] = "level.txt";
            if (read_line(line, sizeof(line)) == LINE_OK && line[0] != '\0')
            {
                snprintf(filename, sizeof(filename), "%s", line);
            }

            ErrorCode err = game_init_from_file(&game, filename);
            if (err != ERR_OK)
            {
                printf("Error loading file '%s': %s\n", filename, error_code_to_string(err));
                printf("Please try again or select another option.\n\n");
                continue;
            }
            break;
        }
        else
        {
            printf("Invalid choice. Please enter 1 or 2.\n");
        }
    }

    game_run(&game);
    game_free(&game);

    printf("Goodbye!\n");
}
