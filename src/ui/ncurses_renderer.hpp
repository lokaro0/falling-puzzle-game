#pragma once

#include <vector>

#include "domain/position.hpp"

namespace puyopuyo {
class Game;
struct Piece;
}

namespace puyopuyo::ui {

class NcursesRenderer final {
public:
    void render(
        const Game& game,
        int high_score,
        const Piece& next_piece,
        const std::vector<Position>& hidden_positions = {}
    ) const;
};

}  // namespace puyopuyo::ui