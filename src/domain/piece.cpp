#include "domain/piece.hpp"

namespace puyopuyo {

Position satellite_position(const Piece& piece) noexcept {
    switch (piece.satellite_direction) {
    case Direction::Up:
        return Position{piece.pivot_position.row - 1, piece.pivot_position.col};
    case Direction::Right:
        return Position{piece.pivot_position.row, piece.pivot_position.col + 1};
    case Direction::Down:
        return Position{piece.pivot_position.row + 1, piece.pivot_position.col};
    case Direction::Left:
        return Position{piece.pivot_position.row, piece.pivot_position.col - 1};
    }

    return piece.pivot_position;
}

Piece moved(Piece piece, int row_delta, int col_delta) noexcept {
    piece.pivot_position.row += row_delta;
    piece.pivot_position.col += col_delta;
    return piece;
}

Piece rotated_clockwise(Piece piece) noexcept {
    switch (piece.satellite_direction) {
    case Direction::Up:
        piece.satellite_direction = Direction::Right;
        break;
    case Direction::Right:
        piece.satellite_direction = Direction::Down;
        break;
    case Direction::Down:
        piece.satellite_direction = Direction::Left;
        break;
    case Direction::Left:
        piece.satellite_direction = Direction::Up;
        break;
    }

    return piece;
}

}  // namespace puyopuyo