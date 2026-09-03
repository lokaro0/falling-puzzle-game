#pragma once

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
    int chain_count = 0;
};

class Game final {
public:
    Game(int rows, int cols);

    [[nodiscard]] const Board& board() const noexcept;
    [[nodiscard]] const std::optional<Piece>& active_piece() const noexcept;
    [[nodiscard]] bool is_game_over() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] ResolutionResult resolve();

    bool try_spawn(Piece piece);
    bool try_move(int row_delta, int col_delta);
    [[nodiscard]] FallResult fall_one_row();
    bool try_rotate_clockwise();
    bool lock_active_piece();

private:
    [[nodiscard]] bool can_place(const Piece& piece) const;
    [[nodiscard]] std::vector<std::vector<Position>> find_erasable_groups() const;
    void apply_gravity();

    Board board_;
    std::optional<Piece> active_piece_;
    bool game_over_ = false;
    int score_ = 0;
};

}  // namespace puyopuyo