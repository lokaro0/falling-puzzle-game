#include "ui/ncurses_session.hpp"

#include <curses.h>

#include <stdexcept>

namespace puyopuyo::ui {

NcursesSession::NcursesSession() {
    if (initscr() == nullptr) {
        throw std::runtime_error("Failed to initialize ncurses.");
    }

    try {
        if (start_color() == ERR
            || noecho() == ERR
            || cbreak() == ERR
            || keypad(stdscr, TRUE) == ERR
            || init_pair(1, COLOR_RED, COLOR_BLACK) == ERR
            || init_pair(2, COLOR_BLUE, COLOR_BLACK) == ERR
            || init_pair(3, COLOR_GREEN, COLOR_BLACK) == ERR
            || init_pair(4, COLOR_YELLOW, COLOR_BLACK) == ERR) {
            throw std::runtime_error("Failed to configure ncurses.");
        }

        curs_set(0);
        timeout(0);
    } catch (...) {
        endwin();
        throw;
    }
}

NcursesSession::~NcursesSession() noexcept {
    endwin();
}

}  // namespace puyopuyo::ui