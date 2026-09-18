#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "domain/board.hpp"
#include "domain/piece.hpp"

namespace puyopuyo {

enum class FallResult {
    NoActivePiece,
    Moved,
    Locked,
};

struct ResolutionResult {
    int erased_count = 0;
    int erased_group_count = 0;
};

class Game final {
public:
    Game(int rows, int cols);

    [[nodiscard]] const Board& board() const noexcept;
    [[nodiscard]] const std::optional<Piece>& active_piece() const noexcept;
    [[nodiscard]] bool is_game_over() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] ResolutionResult resolve(
        const std::function<void(const std::vector<Position>&)>& on_erase_group = {},
        const std::function<void()>& on_gravity_step = {}
    );

    bool try_spawn(Piece piece);
    bool try_move(int row_delta, int col_delta);
    [[nodiscard]] FallResult fall_one_row();
    bool try_rotate_clockwise();
    bool lock_active_piece();
    [[nodiscard]] bool apply_gravity_one_row();

private:
    [[nodiscard]] bool can_place(const Piece& piece) const;
    [[nodiscard]] std::vector<std::vector<Position>> find_erasable_groups() const;

    Board board_;
    std::optional<Piece> active_piece_;
    bool game_over_ = false;
    int score_ = 0;
};

}  // namespace puyopuyo