#pragma once

#include <cstddef>
#include <vector>

#include "domain/color.hpp"
#include "domain/position.hpp"

namespace puyopuyo {

class Board final {
public:
    Board(int rows, int cols);

    [[nodiscard]] int rows() const noexcept;
    [[nodiscard]] int cols() const noexcept;
    [[nodiscard]] bool contains(Position position) const noexcept;

    [[nodiscard]] Color at(Position position) const;
    Color& at(Position position);

    void clear() noexcept;

private:
    [[nodiscard]] std::size_t index(Position position) const noexcept;

    int rows_;
    int cols_;
    std::vector<Color> cells_;
};

}  // namespace puyopuyo