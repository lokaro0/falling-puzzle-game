#include "domain/board.hpp"

#include <algorithm>
#include <stdexcept>

namespace puyopuyo {

Board::Board(int rows, int cols)
    : rows_(rows), cols_(cols), cells_() {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Board dimensions must be positive.");
    }

    cells_.assign(
        static_cast<std::size_t>(rows_) * static_cast<std::size_t>(cols_),
        Color::None);
}

int Board::rows() const noexcept {
    return rows_;
}

int Board::cols() const noexcept {
    return cols_;
}

bool Board::contains(Position position) const noexcept {
    return 0 <= position.row && position.row < rows_
        && 0 <= position.col && position.col < cols_;
}

Color Board::at(Position position) const {
    if (!contains(position)) {
        throw std::out_of_range("Board position is out of range.");
    }

    return cells_[index(position)];
}

Color& Board::at(Position position) {
    if (!contains(position)) {
        throw std::out_of_range("Board position is out of range.");
    }

    return cells_[index(position)];
}

void Board::clear() noexcept {
    std::fill(cells_.begin(), cells_.end(), Color::None);
}

std::size_t Board::index(Position position) const noexcept {
    return static_cast<std::size_t>(position.row)
        * static_cast<std::size_t>(cols_)
        + static_cast<std::size_t>(position.col);
}

}  // namespace puyopuyo