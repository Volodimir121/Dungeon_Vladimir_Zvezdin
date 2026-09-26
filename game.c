#include <stdio.h>
#include <string.h>

#include "game.h"
#include "map.h"

int process_command(const char* command)
{
    if (strcmp(command, "north") == 0)
    {
        printf("You went north.\n");
    }
    else if (strcmp(command, "south") == 0)
    {
        printf("You went south.\n");
    }
    else if (strcmp(command, "east") == 0)
    {
        printf("You went east.\n");
    }
    else if (strcmp(command, "west") == 0)
    {
        printf("You went west.\n");
    }
    else if (strcmp(command, "look") == 0)
    {
        printf("You look around the dungeon.\n");
        print_map();
    }
    else if (strcmp(command, "help") == 0)
    {
        printf("Available commands:\n");
        printf("north - go north\n");
        printf("south - go south\n");
        printf("east  - go east\n");
        printf("west  - go west\n");
        printf("look  - look around the dungeon\n");
        printf("help  - show this help\n");
        printf("quit  - exit the game\n");
    }
    else if (strcmp(command, "quit") == 0)
    {
        return 1;
    }
    else
    {
        printf("Unknown command.\n");
    }

    return 0;
}

void run_game(void)
{
    char command[16];
    int turn = 1;

    printf("Welcome to the Dungeon!\n");

    print_map();

    printf("@ - Player  # - Walls  E - Exit\n\n");

    while (1)
    {
        printf("Turn %d\n", turn);
        printf("> ");

        scanf("%15s", command);

        if (process_command(command))
        {
            printf("Goodbye!\n");
            break;
        }

        turn++;
        printf("\n");
    }
}