#pragma once

namespace puyopuyo {
class Game;
}

namespace puyopuyo::ui {

class NcursesRenderer final {
public:
    void render(const Game& game, int high_score) const;
};

}  // namespace puyopuyo::ui