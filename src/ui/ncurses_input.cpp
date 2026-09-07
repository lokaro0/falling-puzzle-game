#include "ui/ncurses_input.hpp"

#include <curses.h>

namespace puyopuyo::ui {

std::optional<InputAction> NcursesInput::read_action() const noexcept {
    switch (getch()) {
    case KEY_LEFT:
    case 'a':
    case 'A':
        return InputAction::MoveLeft;
    case KEY_RIGHT:
    case 'd':
    case 'D':
        return InputAction::MoveRight;
    case KEY_DOWN:
    case 's':
    case 'S':
        return InputAction::MoveDown;
    case 'r':
    case 'R':
    case 'z':
    case 'Z':
        return InputAction::RotateClockwise;
    case 'q':
    case 'Q':
        return InputAction::Quit;
    default:
        return std::nullopt;
    }
}

}  // namespace puyopuyo::ui