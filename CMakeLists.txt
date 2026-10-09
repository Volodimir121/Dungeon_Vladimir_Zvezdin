cmake_minimum_required(VERSION 4.3)
project(Dungeon_Vladimir_Zvezdin C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(
        ${PROJECT_NAME}
        main.c
        game.c
        game.h
        map.c
        map.h
        statushero.c
        statushero.h
        Hero.c
        Hero.h
        parcer.c
        parcer.h
)
