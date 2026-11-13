cmake_minimum_required(VERSION 3.10)
project(Dungeon_Vladimir_Zvezdin CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(.)

add_executable(
        ${PROJECT_NAME}
        main.cpp
        Map.h
        Map.cpp
        MapRenderer.h
        MapRenderer.cpp
        Creature.h
        Creature.cpp
        Hero.h
        Hero.cpp
        Monster.h
        Monster.cpp
        Game.h
        Game.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/level.txt
               ${CMAKE_CURRENT_BINARY_DIR}/level.txt COPYONLY)
