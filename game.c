#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "hero.h"
#include "map.h"
#include "statushero.h"

/* Указатель hero во всех cmd_* не проверяется на NULL: функции вызываются
   только через process_command из run_game, которая передаёт адрес своей
   локальной переменной (&hero) — он не может быть нулевым. Функции,
   принимающие произвольные указатели (hero.c, map.c), проверяют их сами. */
typedef int (*CommandFn)(Hero* hero);

typedef struct
{
    const char* name;
    const char* help;
    CommandFn fn;
} Command;

/* Если герой стоит на золоте — забирает его и очищает клетку.
   Состояние героя меняется через указатель (add_gold получает &hero->gold).
   hero проверяется на NULL до первого разыменования. */
static void pick_up_gold(Hero* hero)
{
    if (hero == NULL)
    {
        return;
    }

    CellType type;
    int passable;

    if (!map_cell_info(map_grid(), hero->x, hero->y, &type, &passable))
    {
        return;
    }

    if (type == CELL_GOLD)
    {
        add_gold(&hero->gold, 1);
        map_clear(hero->x, hero->y);
        printf("You found gold! Total gold: %u\n", hero->gold);
    }
}


static int move_hero(Hero* hero, int dx, int dy, const char* direction)
{
    if (hero == NULL)
    {
        return 0;
    }

    if (!shift_position(&hero->x, &hero->y, dx, dy))
    {
        printf("You can't go %s - something blocks the way.\n", direction);
        return 0;
    }

    printf("You went %s.\n", direction);

    pick_up_gold(hero);

    return 0;
}

static int cmd_north(Hero* hero)
{
    return move_hero(hero, 0, -1, "north");
}

static int cmd_south(Hero* hero)
{
    return move_hero(hero, 0, 1, "south");
}

static int cmd_east(Hero* hero)
{
    return move_hero(hero, 1, 0, "east");
}

static int cmd_west(Hero* hero)
{
    return move_hero(hero, -1, 0, "west");
}

static int cmd_look(Hero* hero)
{
    printf("You look around the dungeon.\n");
    map_print(hero->x, hero->y);
    return 0;
}

static int cmd_status(Hero* hero)
{
    float health_percent =
        ((float)hero->health / (float)hero->max_health) * 100.0f;

    printf("Health: %d/%d (%.1f%%)\n",
        hero->health, hero->max_health, health_percent);
    printf("Attack: %d\n", hero->attack);
    printf("Gold: %u\n", hero->gold);
    printf("Floor: %u\n", hero->floor);

    print_statuses(hero->states);
    return 0;
}

static int cmd_hit(Hero* hero)
{
    int damage = 0;

    scanf("%d", &damage);

    if (!damage_health(&hero->health, damage))
    {
        printf("Invalid damage value.\n");
        return 0;
    }

    printf("You took %d damage.\n", damage);
    printf("Health: %d/%d\n", hero->health, hero->max_health);
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

/* Показывает тип и проходимость соседних клеток. Тип и признак
   проходимости приходят из map_cell_info через два выходных параметра. */
static int cmd_inspect(Hero* hero)
{
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
        CellType type;
        int passable;

        if (map_cell_info(map_grid(), hero->x + DIRECTIONS[i].dx,
                hero->y + DIRECTIONS[i].dy, &type, &passable))
        {
            printf("%-6s: %s, %s\n", DIRECTIONS[i].name,
                cell_type_name(type), passable ? "passable" : "blocked");
        }
        else
        {
            printf("%-6s: outside the map\n", DIRECTIONS[i].name);
        }
    }

    return 0;
}

static int cmd_addr(Hero* hero)
{
    int local = 0;

    printf("-- game: addresses --\n");
    map_print_addresses();
    printf("local variable (stack)     : %p\n", (void*)&local);
    printf("hero (stack of run_game)   : %p\n", (void*)hero);
    printf("  &hero->gold              : %p\n", (void*)&hero->gold);
    printf("  &hero->x / &hero->y      : %p / %p\n",
        (void*)&hero->x, (void*)&hero->y);

    return 0;
}
static int cmd_check(Hero* hero)
{
    if (map_check_reachable(hero->x, hero->y))
    {
        printf("The exit is reachable from here.\n");
    }
    else
    {
        printf("The exit is NOT reachable from here.\n");
    }

    return 0;
}

static int cmd_quit(Hero* hero)
{
    return 1;
}

static int cmd_help(Hero* hero);

static const Command COMMANDS[] = {
    { "north",  "go north",                 cmd_north  },
    { "south",  "go south",                 cmd_south  },
    { "east",   "go east",                  cmd_east   },
    { "west",   "go west",                  cmd_west   },
    { "look",   "look around the dungeon",  cmd_look   },
    { "status", "show hero status",         cmd_status },
    { "hit",    "take damage (asks amount)",cmd_hit    },
    { "check",  "check if exit is reachable", cmd_check },
    { "inspect", "describe neighbouring cells", cmd_inspect },
    { "addr",   "print memory addresses",   cmd_addr   },
    { "help",   "show this help",           cmd_help   },
    { "quit",   "exit the game",            cmd_quit   },
};

static const size_t COMMAND_COUNT =
    sizeof(COMMANDS) / sizeof(COMMANDS[0]);

static int cmd_help(Hero* hero)
{
    printf("Available commands:\n");

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        printf("%-8s- %s\n", COMMANDS[i].name, COMMANDS[i].help);
    }

    return 0;
}

static int process_command(const char* command, Hero* hero)
{
    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(command, COMMANDS[i].name) == 0)
        {
            return COMMANDS[i].fn(hero);
        }
    }

    if (process_status_command(command, &hero->states))
    {
        return 0;
    }

    printf("Unknown command.\n");
    return 0;
}

void run_game(void)
{
    Hero hero;
    hero_init(&hero);

    char command[16];
    int turn = 1;

    printf("Welcome to the Dungeon!\n");
    map_print(hero.x, hero.y);
    printf("help - For all available commands @ - Player  # - Walls\n\n");

    while (1)
    {
        printf("Turn %d\n", turn);
        printf("> ");

        scanf("%15s", command);

        if (process_command(command, &hero))
        {
            printf("Goodbye!\n");
            break;
        }

        turn++;
        printf("\n");
    }
}