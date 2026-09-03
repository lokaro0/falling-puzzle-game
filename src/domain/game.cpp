#include "domain/game.hpp"

#include <cstddef>
#include <vector>

namespace puyopuyo {

Game::Game(int rows, int cols)
    : board_(rows, cols) {
}

const Board& Game::board() const noexcept {
    return board_;
}

const std::optional<Piece>& Game::active_piece() const noexcept {
    return active_piece_;
}

bool Game::is_game_over() const noexcept {
    return game_over_;
}

int Game::score() const noexcept {
    return score_;
}

bool Game::try_spawn(Piece piece) {
    if (game_over_ || active_piece_.has_value()) {
        return false;
    }

    if (!can_place(piece)) {
        game_over_ = true;
        return false;
    }

    active_piece_ = piece;
    return true;
}

bool Game::try_move(int row_delta, int col_delta) {
    if (!active_piece_.has_value()) {
        return false;
    }

    const Piece candidate = moved(*active_piece_, row_delta, col_delta);
    if (!can_place(candidate)) {
        return false;
    }

    active_piece_ = candidate;
    return true;
}

FallResult Game::fall_one_row() {
    if (!active_piece_.has_value()) {
        return FallResult::NoActivePiece;
    }

    if (try_move(1, 0)) {
        return FallResult::Moved;
    }

    lock_active_piece();
    return FallResult::Locked;
}

bool Game::try_rotate_clockwise() {
    if (!active_piece_.has_value()) {
        return false;
    }

    const Piece candidate = rotated_clockwise(*active_piece_);
    if (!can_place(candidate)) {
        return false;
    }

    active_piece_ = candidate;
    return true;
}

bool Game::lock_active_piece() {
    if (!active_piece_.has_value()) {
        return false;
    }

    const Piece piece = *active_piece_;
    if (!can_place(piece)) {
        return false;
    }

    board_.at(piece.pivot_position) = piece.pivot_color;
    board_.at(satellite_position(piece)) = piece.satellite_color;
    active_piece_.reset();

    return true;
}

ResolutionResult Game::resolve() {
    ResolutionResult result;

    while (true) {
        const auto erasable_groups = find_erasable_groups();

        if (erasable_groups.empty()) {
            break;
        }

        for (const std::vector<Position>& group : erasable_groups) {
            ++result.chain_count;
            result.erased_count += static_cast<int>(group.size());

            for (const Position position : group) {
                board_.at(position) = Color::None;
            }
        }

        apply_gravity();
    }

    score_ += result.erased_count;
    return result;
}

bool Game::can_place(const Piece& piece) const {
    const Position satellite = satellite_position(piece);

    return board_.contains(piece.pivot_position)
        && board_.contains(satellite)
        && board_.at(piece.pivot_position) == Color::None
        && board_.at(satellite) == Color::None;
}

std::vector<std::vector<Position>> Game::find_erasable_groups() const {
    std::vector<std::vector<Position>> erasable_groups;
    std::vector<bool> visited(
        static_cast<std::size_t>(board_.rows())
            * static_cast<std::size_t>(board_.cols()),
        false);

    constexpr int row_offsets[] = {-1, 0, 1, 0};
    constexpr int col_offsets[] = {0, 1, 0, -1};

    for (int row = 0; row < board_.rows(); ++row) {
        for (int col = 0; col < board_.cols(); ++col) {
            const Position start{row, col};
            const std::size_t start_index =
                static_cast<std::size_t>(row)
                    * static_cast<std::size_t>(board_.cols())
                + static_cast<std::size_t>(col);

            if (visited[start_index] || board_.at(start) == Color::None) {
                continue;
            }

            const Color color = board_.at(start);
            std::vector<Position> group;
            std::vector<Position> pending_positions{start};
            visited[start_index] = true;

            while (!pending_positions.empty()) {
                const Position current = pending_positions.back();
                pending_positions.pop_back();
                group.push_back(current);

                for (int direction = 0; direction < 4; ++direction) {
                    const Position neighbor{
                        current.row + row_offsets[direction],
                        current.col + col_offsets[direction],
                    };

                    if (!board_.contains(neighbor)) {
                        continue;
                    }

                    const std::size_t neighbor_index =
                        static_cast<std::size_t>(neighbor.row)
                            * static_cast<std::size_t>(board_.cols())
                        + static_cast<std::size_t>(neighbor.col);

                    if (visited[neighbor_index]
                        || board_.at(neighbor) != color) {
                        continue;
                    }

                    visited[neighbor_index] = true;
                    pending_positions.push_back(neighbor);
                }
            }

            if (group.size() >= 4U) {
                erasable_groups.push_back(group);
            }
        }
    }

    return erasable_groups;
}

void Game::apply_gravity() {
    for (int col = 0; col < board_.cols(); ++col) {
        int write_row = board_.rows() - 1;

        for (int read_row = board_.rows() - 1; read_row >= 0; --read_row) {
            const Position read_position{read_row, col};
            const Color color = board_.at(read_position);

            if (color == Color::None) {
                continue;
            }

            const Position write_position{write_row, col};
            if (write_position.row != read_position.row) {
                board_.at(write_position) = color;
                board_.at(read_position) = Color::None;
            }

            --write_row;
        }
    }
}

}  // namespace puyopuyo