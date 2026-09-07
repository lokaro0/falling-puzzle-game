#pragma once

#include <optional>

namespace puyopuyo::ui {

enum class InputAction {
    MoveLeft,
    MoveRight,
    MoveDown,
    RotateClockwise,
    Quit,
};

class NcursesInput final {
public:
    [[nodiscard]] std::optional<InputAction> read_action() const noexcept;
};

}  // namespace puyopuyo::ui