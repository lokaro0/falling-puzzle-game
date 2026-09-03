#include "domain/game.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

using puyopuyo::Color;
using puyopuyo::Direction;
using puyopuyo::Game;
using puyopuyo::Piece;
using puyopuyo::Position;
using puyopuyo::FallResult;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Piece create_piece(Position pivot_position, Direction direction) {
    return Piece{
        pivot_position,
        Color::Red,
        Color::Blue,
        direction,
    };
}

Piece create_piece(
    Position pivot_position,
    Direction direction,
    Color pivot_color,
    Color satellite_color) {
    return Piece{
        pivot_position,
        pivot_color,
        satellite_color,
        direction,
    };
}

void spawn_and_lock(Game& game, Piece piece) {
    require(game.try_spawn(piece), "A valid piece must spawn.");
    require(game.lock_active_piece(), "An active piece must lock.");
}

void test_new_game_has_an_empty_board() {
    const Game game(2, 3);

    require(game.board().rows() == 2, "Game board row count is incorrect.");
    require(game.board().cols() == 3, "Game board column count is incorrect.");
    require(!game.active_piece().has_value(), "A new game must have no active piece.");
    require(!game.is_game_over(), "A new game must not be over.");
}

void test_spawn_and_move() {
    Game game(4, 4);

    require(
        game.try_spawn(create_piece(Position{0, 1}, Direction::Right)),
        "A valid piece must spawn.");
    require(game.active_piece().has_value(), "Spawned piece is missing.");

    require(game.try_move(1, 0), "Piece must move down.");
    require(
        game.active_piece()->pivot_position.row == 1
            && game.active_piece()->pivot_position.col == 1,
        "Moved pivot position is incorrect.");

    require(!game.try_move(0, 3), "A piece must not move beyond the right wall.");
    require(
        game.active_piece()->pivot_position.col == 1,
        "Rejected movement must preserve the active piece.");
}

void test_rotation_is_rejected_when_the_satellite_would_leave_the_board() {
    Game game(3, 3);

    require(
        game.try_spawn(create_piece(Position{0, 0}, Direction::Right)),
        "A valid piece must spawn.");
    require(game.try_rotate_clockwise(), "Right to Down rotation must succeed.");
    require(
        !game.try_rotate_clockwise(),
        "Down to Left rotation at the left wall must be rejected.");
    require(
        game.active_piece()->satellite_direction == Direction::Down,
        "Rejected rotation must preserve the active piece.");
}

void test_lock_places_both_colors_on_the_board() {
    Game game(3, 4);

    require(
        game.try_spawn(create_piece(Position{1, 1}, Direction::Right)),
        "A valid piece must spawn.");
    require(game.lock_active_piece(), "An active piece must lock.");
    require(!game.active_piece().has_value(), "Locked piece must no longer be active.");

    require(
        game.board().at(Position{1, 1}) == Color::Red,
        "Pivot color was not written to the board.");
    require(
        game.board().at(Position{1, 2}) == Color::Blue,
        "Satellite color was not written to the board.");
}

void test_fall_moves_then_locks_a_piece() {
    Game game(3, 4);

    require(
        game.try_spawn(create_piece(Position{1, 1}, Direction::Right)),
        "A valid piece must spawn.");

    require(
        game.fall_one_row() == FallResult::Moved,
        "A piece with space below must move down.");
    require(
        game.active_piece()->pivot_position.row == 2,
        "Falling piece pivot did not move down.");

    require(
        game.fall_one_row() == FallResult::Locked,
        "A piece at the bottom must lock.");
    require(!game.active_piece().has_value(), "Locked piece must no longer be active.");

    require(
        game.board().at(Position{2, 1}) == Color::Red,
        "Falling pivot color was not written to the board.");
    require(
        game.board().at(Position{2, 2}) == Color::Blue,
        "Falling satellite color was not written to the board.");

    require(
        game.fall_one_row() == FallResult::NoActivePiece,
        "A game without an active piece must report NoActivePiece.");
}

void test_blocked_spawn_ends_the_game() {
    Game game(3, 4);

    require(
        game.try_spawn(create_piece(Position{1, 1}, Direction::Right)),
        "A valid piece must spawn.");
    require(game.lock_active_piece(), "An active piece must lock.");

    require(
        !game.try_spawn(create_piece(Position{1, 1}, Direction::Right)),
        "A piece must not spawn on occupied cells.");
    require(game.is_game_over(), "A blocked spawn must end the game.");
}

void test_resolve_does_not_erase_a_group_of_three() {
    Game game(4, 4);

    spawn_and_lock(
        game,
        create_piece(
            Position{3, 0},
            Direction::Right,
            Color::Red,
            Color::Red));

    spawn_and_lock(
        game,
        create_piece(
            Position{2, 0},
            Direction::Right,
            Color::Red,
            Color::Blue));

    const puyopuyo::ResolutionResult result = game.resolve();

    require(result.erased_count == 0, "Three puyos must not be erased.");
    require(result.chain_count == 0, "No group must mean no chain count.");
    require(game.score() == 0, "No erased puyos must add no score.");

    require(
        game.board().at(Position{2, 0}) == Color::Red,
        "The first red puyo must remain on the board.");
    require(
        game.board().at(Position{3, 0}) == Color::Red,
        "The second red puyo must remain on the board.");
    require(
        game.board().at(Position{3, 1}) == Color::Red,
        "The third red puyo must remain on the board.");
}

void test_resolve_counts_each_erased_group() {
    Game game(4, 4);

    spawn_and_lock(
        game,
        create_piece(
            Position{2, 0},
            Direction::Right,
            Color::Red,
            Color::Red));
    spawn_and_lock(
        game,
        create_piece(
            Position{3, 0},
            Direction::Right,
            Color::Red,
            Color::Red));

    spawn_and_lock(
        game,
        create_piece(
            Position{2, 2},
            Direction::Right,
            Color::Blue,
            Color::Blue));
    spawn_and_lock(
        game,
        create_piece(
            Position{3, 2},
            Direction::Right,
            Color::Blue,
            Color::Blue));

    const puyopuyo::ResolutionResult result = game.resolve();

    require(result.erased_count == 8, "Two groups must erase eight puyos.");
    require(
        result.chain_count == 2,
        "Two erased groups must increment the chain count twice.");
    require(game.score() == 8, "Erased puyos must be added to the score.");

    require(
        game.board().at(Position{2, 0}) == Color::None,
        "The red group must be erased.");
    require(
        game.board().at(Position{3, 3}) == Color::None,
        "The blue group must be erased.");
}

void test_resolve_counts_groups_created_after_gravity() {
    Game game(4, 4);

    spawn_and_lock(
        game,
        create_piece(
            Position{0, 0},
            Direction::Right,
            Color::Blue,
            Color::Blue));

    spawn_and_lock(
        game,
        create_piece(
            Position{1, 0},
            Direction::Right,
            Color::Red,
            Color::Red));

    spawn_and_lock(
        game,
        create_piece(
            Position{1, 2},
            Direction::Right,
            Color::Red,
            Color::Red));

    spawn_and_lock(
        game,
        create_piece(
            Position{2, 0},
            Direction::Right,
            Color::Blue,
            Color::Blue));

    const puyopuyo::ResolutionResult result = game.resolve();

    require(result.erased_count == 8, "Two erased groups must erase eight puyos.");
    require(
        result.chain_count == 2,
        "A group erased after gravity must increment the chain count.");
    require(game.score() == 8, "All erased puyos must be added to the score.");

    require(
        game.board().at(Position{3, 0}) == Color::None,
        "The blue group created after gravity must be erased.");
}

void test_fall_is_rejected_when_a_fixed_piece_is_below() {
    Game game(4, 4);

    spawn_and_lock(
        game,
        create_piece(
            Position{3, 0},
            Direction::Right,
            Color::Red,
            Color::Blue));

    require(
        game.try_spawn(create_piece(Position{2, 0}, Direction::Right)),
        "A piece above fixed puyos must spawn.");

    require(
        !game.try_move(1, 0),
        "A piece must not move onto fixed puyos.");

    require(
        game.active_piece()->pivot_position.row == 2,
        "Rejected movement must preserve the pivot position.");
    require(
        game.active_piece()->pivot_position.col == 0,
        "Rejected movement must preserve the pivot column.");
}

}  // namespace

int main() {
    try {
        test_new_game_has_an_empty_board();
        test_spawn_and_move();
        test_rotation_is_rejected_when_the_satellite_would_leave_the_board();
        test_lock_places_both_colors_on_the_board();
        test_fall_moves_then_locks_a_piece();
        test_blocked_spawn_ends_the_game();
        test_resolve_does_not_erase_a_group_of_three();
        test_resolve_counts_each_erased_group();
        test_resolve_counts_groups_created_after_gravity();
        test_fall_is_rejected_when_a_fixed_piece_is_below();
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All Game tests passed.\n";
    return 0;
}