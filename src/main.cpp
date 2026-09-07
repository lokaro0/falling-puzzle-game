#include <chrono>
#include <random>
#include <thread>

#include "domain/game.hpp"
#include "storage/high_score_store.hpp"
#include "ui/ncurses_input.hpp"
#include "ui/ncurses_renderer.hpp"
#include "ui/ncurses_session.hpp"

namespace {

constexpr int kBoardRows = 12;
constexpr int kBoardCols = 6;
constexpr std::chrono::milliseconds kFallInterval{500};
constexpr std::chrono::milliseconds kFrameInterval{10};

puyopuyo::Color random_color(std::mt19937& engine) {
    std::uniform_int_distribution<int> distribution(
        static_cast<int>(puyopuyo::Color::Red),
        static_cast<int>(puyopuyo::Color::Yellow)
    );

    return static_cast<puyopuyo::Color>(distribution(engine));
}

puyopuyo::Piece make_random_piece(
    const puyopuyo::Game& game,
    std::mt19937& engine
) {
    return puyopuyo::Piece{
        puyopuyo::Position{0, game.board().cols() / 2},
        random_color(engine),
        random_color(engine),
        puyopuyo::Direction::Down,
    };
}

void update_high_score(
    const puyopuyo::Game& game,
    int& high_score,
    const puyopuyo::HighScoreStore& high_score_store
) {
    if (game.score() <= high_score) {
        return;
    }

    high_score = game.score();
    high_score_store.save(high_score);
}

void advance_game(
    puyopuyo::Game& game, 
    std::mt19937& engine, 
    int& high_score,
    const puyopuyo::HighScoreStore& high_score_store
) {
    if (game.fall_one_row() != puyopuyo::FallResult::Locked) {
        return;
    }

    static_cast<void>(game.resolve());
    update_high_score(game, high_score, high_score_store);
    game.try_spawn(make_random_piece(game, engine));
}

}  // namespace

int main() {
    puyopuyo::ui::NcursesSession session;
    puyopuyo::ui::NcursesInput input;
    puyopuyo::ui::NcursesRenderer renderer;

    puyopuyo::Game game(kBoardRows, kBoardCols);
    const puyopuyo::HighScoreStore high_score_store{"data/high_score.txt"};
    int high_score = high_score_store.load();

    std::random_device random_device;
    std::mt19937 engine(random_device());

    game.try_spawn(make_random_piece(game, engine));

    auto last_fall_time = std::chrono::steady_clock::now();
    bool should_quit = false;

    while (!should_quit) {
        renderer.render(game, high_score);

        const auto action = input.read_action();

        if (action.has_value()) {
            switch (*action) {
            case puyopuyo::ui::InputAction::MoveLeft:
                game.try_move(0, -1);
                break;

            case puyopuyo::ui::InputAction::MoveRight:
                game.try_move(0, 1);
                break;

            case puyopuyo::ui::InputAction::MoveDown:
                advance_game(game, engine, high_score, high_score_store);
                last_fall_time = std::chrono::steady_clock::now();
                break;

            case puyopuyo::ui::InputAction::RotateClockwise:
                game.try_rotate_clockwise();
                break;

            case puyopuyo::ui::InputAction::Quit:
                should_quit = true;
                break;
            }
        }

        if (!game.is_game_over()) {
            const auto current_time = std::chrono::steady_clock::now();

            if (current_time - last_fall_time >= kFallInterval) {
                advance_game(game, engine, high_score, high_score_store);
                last_fall_time = current_time;
            }
        }

        std::this_thread::sleep_for(kFrameInterval);
    }

    return 0;
}