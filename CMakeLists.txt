cmake_minimum_required(VERSION 3.20)

project(falling_puzzle_game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Curses REQUIRED)

add_executable(falling-puzzle
    src/puyo.cpp
)

target_include_directories(falling-puzzle PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(falling-puzzle PRIVATE ${CURSES_LIBRARIES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(falling-puzzle PRIVATE -Wall -Wextra -Wpedantic)
endif()