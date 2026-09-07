#pragma once

namespace puyopuyo::ui {

class NcursesSession final {
public:
    NcursesSession();
    ~NcursesSession() noexcept;

    NcursesSession(const NcursesSession&) = delete;
    NcursesSession& operator=(const NcursesSession&) = delete;
    NcursesSession(NcursesSession&&) = delete;
    NcursesSession& operator=(NcursesSession&&) = delete;
};

}  // namespace puyopuyo::ui