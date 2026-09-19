cmake_minimum_required(VERSION 4.3)
project(Dungeon_Vladimir_Zvezdin C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(
        ${PROJECT_NAME}
        main.c
)
