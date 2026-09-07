#include "ui/ncurses_renderer.hpp"

#include <curses.h>

#include <optional>

#include "domain/game.hpp"
#include "domain/piece.hpp"

namespace puyopuyo::ui {
namespace {

constexpr int kBoardTop = 1;
constexpr int kBoardLeft = 0;
constexpr int kCellTop = kBoardTop + 1;
constexpr int kCellLeft = kBoardLeft + 2;

bool same_position(Position left, Position right) noexcept {
    return left.row == right.row && left.col == right.col;
}

int color_pair(Color color) noexcept {
    switch (color) {
    case Color::None:
        return 0;
    case Color::Red:
        return 1;
    case Color::Blue:
        return 2;
    case Color::Green:
        return 3;
    case Color::Yellow:
        return 4;
    }

    return 0;
}

chtype cell_symbol(Color color) noexcept {
    switch (color) {
    case Color::None:
        return '.';
    case Color::Red:
        return 'R';
    case Color::Blue:
        return 'B';
    case Color::Green:
        return 'G';
    case Color::Yellow:
        return 'Y';
    }

    return '?';
}

void draw_rectangle(int top, int left, int bottom, int right) {
    mvhline(top, left, ACS_HLINE, right - left);
    mvhline(bottom, left, ACS_HLINE, right - left);
    mvvline(top, left, ACS_VLINE, bottom - top);
    mvvline(top, right, ACS_VLINE, bottom - top);

    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, right, ACS_URCORNER);
    mvaddch(bottom, left, ACS_LLCORNER);
    mvaddch(bottom, right, ACS_LRCORNER);
}

Color displayed_color(const Game& game, Position position) {
    const std::optional<Piece>& active_piece = game.active_piece();

    if (active_piece.has_value()) {
        if (same_position(position, active_piece->pivot_position)) {
            return active_piece->pivot_color;
        }

        if (same_position(position, satellite_position(*active_piece))) {
            return active_piece->satellite_color;
        }
    }

    return game.board().at(position);
}

}  // namespace

void NcursesRenderer::render(const Game& game, int high_score) const {
    erase();

    const Board& board = game.board();
    draw_rectangle(
        kBoardTop,
        kBoardLeft,
        kBoardTop + board.rows() + 1,
        kBoardLeft + board.cols() * 2 + 2);

    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            const Position position{row, col};
            const Color color = displayed_color(game, position);

            attrset(COLOR_PAIR(color_pair(color)));
            mvaddch(kCellTop + row, kCellLeft + col * 2, cell_symbol(color));
        }
    }

    attrset(COLOR_PAIR(0));
    mvprintw(0, 1, "Score: %d", game.score());
    mvprintw(0, 17, "High score: %d", high_score);
    if (game.is_game_over()) {
        mvaddstr(4, 17, "Game over");
        mvaddstr(5, 17, "Press Q to quit");
    }else {
        mvaddstr(4, 17, "Arrow keys / A,D,S: Move");
        mvaddstr(5, 17, "R or Z: Rotate  Q: Quit");
    }

    refresh();
}

}  // namespace puyopuyo::ui