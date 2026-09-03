#pragma once

#include "domain/color.hpp"
#include "domain/position.hpp"

namespace puyopuyo {

enum class Direction {
    Up,
    Right,
    Down,
    Left,
};

struct Piece {
    Position pivot_position;
    Color pivot_color;
    Color satellite_color;
    Direction satellite_direction;
};

[[nodiscard]] Position satellite_position(const Piece& piece) noexcept;

[[nodiscard]] Piece moved(
    Piece piece,
    int row_delta,
    int col_delta) noexcept;

[[nodiscard]] Piece rotated_clockwise(Piece piece) noexcept;

}  // namespace puyopuyo