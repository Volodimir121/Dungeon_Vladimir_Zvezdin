#include <stdio.h>

#include "map.h"

void print_map(void)
{
    const char* map[] = {
        "####################",
        "# @    #          E#",
        "#      #           #",
        "#      #           #",
        "#                  #",
        "#                  #",
        "####################"
    };

    const size_t size = sizeof(map) / sizeof(map[0]);

    for (size_t i = 0; i < size; i++)
    {
        printf("%s\n", map[i]);
    }
}