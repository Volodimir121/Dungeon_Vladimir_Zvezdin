#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "Hero.h"
#include "map.h"
#include "parser.h"
#include "statushero.h"

/* Команда получает героя и свой единственный необязательный аргумент
   (NULL, если аргумента не было). Возвращает 1, чтобы завершить игру. */
typedef int (*CommandFn)(Hero* hero, const char* arg);

typedef struct
{
    const char* name;   /* полное имя: "north" */
    const char* alias;  /* сокращение: "n" (пустая строка — нет) */
    const char* help;
    int max_args;       /* сколько аргументов допускает команда */
    int is_move;        /* можно ли писать "go <имя>" */
    CommandFn fn;
} Command;

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

static int cmd_north(Hero* hero, const char* arg)
{
    return move_hero(hero, 0, -1, "north");
}

static int cmd_south(Hero* hero, const char* arg)
{
    return move_hero(hero, 0, 1, "south");
}

static int cmd_east(Hero* hero, const char* arg)
{
    return move_hero(hero, 1, 0, "east");
}

static int cmd_west(Hero* hero, const char* arg)
{
    return move_hero(hero, -1, 0, "west");
}

static int cmd_look(Hero* hero, const char* arg)
{
    printf("You look around the dungeon.\n");
    map_print(hero->x, hero->y);
    return 0;
}

static int cmd_status(Hero* hero, const char* arg)
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

/* Превращает текстовый аргумент команды в число и сама объясняет игроку,
   что не так. Возвращает 1, если *out заполнено, иначе 0 (сообщение уже
   напечатано). what — как называется аргумент, для текста сообщений. */
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

static int cmd_hit(Hero* hero, const char* arg)
{
    unsigned int damage;

    if (!read_number_arg(arg, "damage", INT_MAX, &damage))
    {
        return 0;
    }

    if (!damage_health(&hero->health, (int)damage))
    {
        printf("Invalid damage value.\n");
        return 0;
    }

    printf("You took %u damage.\n", damage);
    printf("Health: %d/%d\n", hero->health, hero->max_health);
    return 0;
}

/* Инвентаря в игре пока нет, поэтому команда только проверяет номер
   предмета и сообщает о нём. */
static int cmd_use(Hero* hero, const char* arg)
{
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

static int cmd_inspect(Hero* hero, const char* arg)
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

static int cmd_addr(Hero* hero, const char* arg)
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

static int cmd_check(Hero* hero, const char* arg)
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

static int cmd_quit(Hero* hero, const char* arg)
{
    return 1;
}

static int cmd_help(Hero* hero, const char* arg);

static const Command COMMANDS[] = {
    { "north",   "n",  "go north",                   0, 1, cmd_north   },
    { "south",   "s",  "go south",                   0, 1, cmd_south   },
    { "east",    "e",  "go east",                    0, 1, cmd_east    },
    { "west",    "w",  "go west",                    0, 1, cmd_west    },
    { "look",    "l",  "look around the dungeon",    0, 0, cmd_look    },
    { "status",  "st", "show hero status",           0, 0, cmd_status  },
    { "hit",     "",   "take damage: hit N",         1, 0, cmd_hit     },
    { "use",     "u",  "use an item: use N",         1, 0, cmd_use     },
    { "check",   "c",  "check if exit is reachable", 0, 0, cmd_check   },
    { "inspect", "i",  "describe neighbouring cells",0, 0, cmd_inspect },
    { "addr",    "",   "print memory addresses",     0, 0, cmd_addr    },
    { "help",    "h",  "show this help",             0, 0, cmd_help    },
    { "quit",    "q",  "exit the game",              0, 0, cmd_quit    },
};

static const size_t COMMAND_COUNT =
    sizeof(COMMANDS) / sizeof(COMMANDS[0]);

static int cmd_help(Hero* hero, const char* arg)
{
    printf("Available commands (name, short form):\n");

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        printf("%-8s %-3s - %s\n",
            COMMANDS[i].name, COMMANDS[i].alias, COMMANDS[i].help);
    }

    printf("go <dir>     - same as north/south/east/west, e.g. go north\n");

    return 0;
}

/* Ищет команду по полному имени или по сокращению. Слова сравниваются
   через strcmp (по содержимому), а не через == (это было бы сравнение
   адресов). Возвращает NULL, если такой команды нет. */
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

/* Выполняет уже разбитую на слова команду: argv[0] — первое слово,
   argc >= 1. Возвращает 1, если игру нужно завершить. */
static int process_command(Hero* hero, int argc, char* argv[])
{
    int first = 0;       /* индекс слова, которое называет команду */
    int only_moves = 0;  /* после "go" допустимы только направления */

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
        if (process_status_command(argv[0], &hero->states))
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

    return cmd->fn(hero, arg_count > 0 ? argv[first + 1] : NULL);
}

void run_game(void)
{
    Hero hero;
    hero_init(&hero);

    char line[LINE_BUFFER_SIZE];
    char* words[MAX_WORDS];
    int turn = 1;

    printf("Welcome to the Dungeon!\n");
    map_print(hero.x, hero.y);
    printf("help - For all available commands @ - Player  # - Walls\n\n");

    while (1)
    {
        printf("Turn %d\n", turn);
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

        if (process_command(&hero, count, words))
        {
            break;
        }

        turn++;
        printf("\n");
    }

    printf("Goodbye!\n");
}