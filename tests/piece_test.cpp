#include "domain/piece.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

using puyopuyo::Color;
using puyopuyo::Direction;
using puyopuyo::Piece;
using puyopuyo::Position;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_position(
    Position actual,
    int expected_row,
    int expected_col,
    const char* message) {
    require(
        actual.row == expected_row && actual.col == expected_col,
        message);
}

Piece create_piece(Direction direction) {
    return Piece{
        Position{3, 2},
        Color::Red,
        Color::Blue,
        direction,
    };
}

void test_satellite_position_for_each_direction() {
    require_position(
        puyopuyo::satellite_position(create_piece(Direction::Up)),
        2,
        2,
        "Up satellite position is incorrect.");

    require_position(
        puyopuyo::satellite_position(create_piece(Direction::Right)),
        3,
        3,
        "Right satellite position is incorrect.");

    require_position(
        puyopuyo::satellite_position(create_piece(Direction::Down)),
        4,
        2,
        "Down satellite position is incorrect.");

    require_position(
        puyopuyo::satellite_position(create_piece(Direction::Left)),
        3,
        1,
        "Left satellite position is incorrect.");
}

void test_move_returns_a_new_piece() {
    const Piece original = create_piece(Direction::Right);
    const Piece moved_piece = puyopuyo::moved(original, 1, -1);

    require_position(
        moved_piece.pivot_position,
        4,
        1,
        "Moved pivot position is incorrect.");
    require_position(
        puyopuyo::satellite_position(moved_piece),
        4,
        2,
        "Moved satellite position is incorrect.");

    require_position(
        original.pivot_position,
        3,
        2,
        "moved() must not modify the original piece.");
}

void test_clockwise_rotation_keeps_the_pivot() {
    const Piece original = create_piece(Direction::Right);
    const Piece rotated_piece = puyopuyo::rotated_clockwise(original);

    require(
        rotated_piece.satellite_direction == Direction::Down,
        "Clockwise rotation from Right must face Down.");
    require_position(
        rotated_piece.pivot_position,
        3,
        2,
        "Rotation must not move the pivot.");
    require_position(
        puyopuyo::satellite_position(rotated_piece),
        4,
        2,
        "Rotated satellite position is incorrect.");
    require(
        original.satellite_direction == Direction::Right,
        "rotated_clockwise() must not modify the original piece.");
}

void test_four_rotations_restore_the_original_direction() {
    Piece piece = create_piece(Direction::Up);

    for (int count = 0; count < 4; ++count) {
        piece = puyopuyo::rotated_clockwise(piece);
    }

    require(
        piece.satellite_direction == Direction::Up,
        "Four clockwise rotations must restore the original direction.");
    require_position(
        puyopuyo::satellite_position(piece),
        2,
        2,
        "Four clockwise rotations must restore the satellite position.");
}

}  // namespace

int main() {
    try {
        test_satellite_position_for_each_direction();
        test_move_returns_a_new_piece();
        test_clockwise_rotation_keeps_the_pivot();
        test_four_rotations_restore_the_original_direction();
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All Piece tests passed.\n";
    return 0;
}