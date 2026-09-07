cmake_minimum_required(VERSION 3.20)

project(puyopuyo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(enable_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

find_package(Curses REQUIRED)

add_library(puyopuyo_ui
    src/ui/ncurses_input.cpp
    src/ui/ncurses_renderer.cpp
    src/ui/ncurses_session.cpp
)

target_include_directories(puyopuyo_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
        ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(puyopuyo_ui
    PRIVATE
        ${CURSES_LIBRARIES}
)

enable_warnings(puyopuyo_ui)

add_library(puyopuyo_domain
    src/domain/board.cpp
    src/domain/game.cpp
    src/domain/piece.cpp
)

target_include_directories(puyopuyo_domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

enable_warnings(puyopuyo_domain)

add_library(puyopuyo_storage
    src/storage/high_score_store.cpp
)

target_include_directories(puyopuyo_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

enable_warnings(puyopuyo_storage)

add_executable(puyopuyo
    src/main.cpp
)

target_link_libraries(puyopuyo
    PRIVATE
        ${CURSES_LIBRARIES}
        puyopuyo_domain
        puyopuyo_ui
        puyopuyo_storage
)

enable_warnings(puyopuyo)

include(CTest)

if(BUILD_TESTING)
    add_executable(puyopuyo_board_tests
        tests/board_test.cpp
    )

    target_link_libraries(puyopuyo_board_tests
        PRIVATE
            puyopuyo_domain
    )

    enable_warnings(puyopuyo_board_tests)

    add_test(
        NAME puyopuyo_board_tests
        COMMAND puyopuyo_board_tests
    )

    add_executable(puyopuyo_piece_tests
        tests/piece_test.cpp
    )

    target_link_libraries(puyopuyo_piece_tests
        PRIVATE
            puyopuyo_domain
    )

    enable_warnings(puyopuyo_piece_tests)

    add_test(
        NAME puyopuyo_piece_tests
        COMMAND puyopuyo_piece_tests
    )

    add_executable(puyopuyo_game_tests
        tests/game_test.cpp
    )

    target_link_libraries(puyopuyo_game_tests
        PRIVATE
            puyopuyo_domain
    )

    enable_warnings(puyopuyo_game_tests)

    add_test(
        NAME puyopuyo_game_tests
        COMMAND puyopuyo_game_tests
    )

    add_executable(puyopuyo_high_score_store_tests
        tests/high_score_store_test.cpp
    )

    target_link_libraries(puyopuyo_high_score_store_tests
        PRIVATE
            puyopuyo_storage
    )

    enable_warnings(puyopuyo_high_score_store_tests)

    add_test(
        NAME puyopuyo_high_score_store_tests
        COMMAND puyopuyo_high_score_store_tests
    )

endif()