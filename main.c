#include <stdio.h>
#include "game.h"

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        Game game;
        ErrorCode code = game_init_from_file(&game, argv[1]);
        if (code != ERR_OK)
        {
            printf("Error loading file '%s': %s\n", argv[1], error_code_to_string(code));
            return (int)code;
        }

        game_run(&game);
        game_free(&game);
        return 0;
    }

    run_game();
    return 0;
}
