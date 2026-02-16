cmake_minimum_required(VERSION 3.10)
project(game C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -O2 -fsanitize=address)
add_link_options(-fsanitize=address)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(tui src/tui.c)
add_library(input src/input.c)
add_library(table src/table.c)
add_library(poker src/poker.c)
add_library(menu src/menu.c)

add_executable(game main.c)

target_link_libraries(game poker tui input table menu)
