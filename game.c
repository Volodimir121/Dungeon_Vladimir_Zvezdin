#include <stdio.h>
#include <string.h>

#include "game.h"
#include "map.h"

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
        }
        else if (strcmp(command, "quit") == 0)
        {
            printf("Goodbye!\n");
            break;
        }
        else
        {
            printf("Unknown command. Available commands: "
                   "north, south, east, west, look, quit.\n");
        }

        turn++;
        printf("\n");
    }
}