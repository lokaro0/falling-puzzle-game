#include "domain/game.hpp"

#include <cstddef>
#include <vector>
#include <algorithm>
#include <limits>

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

int Game::current_chain() const noexcept {
    return current_chain_;
}

bool Game::try_spawn(Piece piece) {
    if (game_over_ || active_piece_.has_value()) {
        return false;
    }

    if (can_place(piece)) {
        active_piece_ = piece;
        return true;
    }

    if (piece.pivot_position.row == 0
        && piece.satellite_direction == Direction::Right) {
            for (int col = 0; col < board_.cols() - 1; ++col) {
                piece.pivot_position = Position{0, col};

                if (can_place(piece)) {
                    active_piece_ = piece;
                    return true;
                }
            }
        }
    game_over_ = true;
    return false;
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

ResolutionResult Game::erase_once(
    const std::function<void(const std::vector<Position>&)>&
        on_erase_group
) {
    return erase_once(find_erasable_groups(), on_erase_group);
}

ResolutionResult Game::erase_once(
    const std::vector<std::vector<Position>>& groups,
    const std::function<void(const std::vector<Position>&)>&
        on_erase_group
) {
    ResolutionResult result;

    if (groups.empty()) {
        current_chain_ = 0;
        return result;
    }

    ++current_chain_;
    std::vector<int> group_scores;

    for (const auto& group : groups) {
        if (on_erase_group) {
            on_erase_group(group);
        }

        ++result.erased_group_count;
        result.erased_count += static_cast<int>(group.size());

        for (const Position position : group) {
            board_.at(position) = Color::None;
        }

        const int count = static_cast<int>(group.size());

        int group_score = 4;
        for (int i = 5; i <= count; ++i) {
            group_score += i - 3;
        }

        group_scores.push_back(group_score);
    }

    std::sort(group_scores.begin(), group_scores.end());

    int step_score = 0;
    for (std::size_t i = 0; i < group_scores.size(); ++i) {
        step_score += group_scores[i] * static_cast<int>(i + 1);
    }

    const int max_score = std::numeric_limits<int>::max();

    for (int chain = 1; chain < current_chain_; ++chain) {
        if (step_score > max_score / 2) {
            step_score = max_score;
            break;
        }

        step_score *= 2;
    }

    score_ += std::min(step_score, max_score - score_);
    return result;
}

ResolutionResult Game::resolve(
    const std::function<void(const std::vector<Position>&)>&
        on_erase_group,
    const std::function<void()>& on_gravity_step
) {
    ResolutionResult result;

    while (true) {
        const auto step = erase_once(on_erase_group);

        if (step.erased_count == 0) {
            break;
        }

        result.erased_count += step.erased_count;
        result.erased_group_count += step.erased_group_count;

        while (apply_gravity_one_row()) {
            if (on_gravity_step) {
                on_gravity_step();
            }
        }
    }

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

bool Game::apply_gravity_one_row() {
    bool moved = false;

    for (int row = board_.rows() - 2; row >= 0; --row) {
        for (int col = 0; col < board_.cols(); ++col) {
            const Position current{row, col};
            const Position below{row + 1, col};

            if (board_.at(current) == Color::None
                || board_.at(below) != Color::None) {
                continue;
            }

            board_.at(below) = board_.at(current);
            board_.at(current) = Color::None;
            moved = true;
        }
    }

    return moved;
}

}  // namespace puyopuyo