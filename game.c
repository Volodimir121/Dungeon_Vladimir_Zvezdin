#include <stdio.h>
#include <string.h>

#include "game.h"
#include "hero.h"
#include "map.h"
#include "statushero.h"

typedef int (*CommandFn)(Hero* hero);

typedef struct
{
    const char* name;
    const char* help;
    CommandFn fn;
} Command;

static int cmd_north(Hero* hero)
{

    printf("You went north.\n");
    return 0;
}

static int cmd_south(Hero* hero)
{
    printf("You went south.\n");
    return 0;
}

static int cmd_east(Hero* hero)
{
    printf("You went east.\n");
    return 0;
}

static int cmd_west(Hero* hero)
{
    printf("You went west.\n");
    return 0;
}

static int cmd_look(Hero* hero)
{
    printf("You look around the dungeon.\n");
    print_map();
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
    int damage;

    scanf("%d", &damage);

    hero_take_damage(hero, damage);

    printf("You took %d damage.\n", damage);
    printf("Health: %d/%d\n", hero->health, hero->max_health);
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

void run_game()
{
    Hero hero;
    hero_init(&hero);

    char command[16];
    int turn = 1;

    printf("Welcome to the Dungeon!\n");
    print_map();
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