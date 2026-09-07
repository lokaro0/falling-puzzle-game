#pragma once

#include <filesystem>

namespace puyopuyo {

class HighScoreStore final {
public:
    explicit HighScoreStore(std::filesystem::path file_path);

    [[nodiscard]] int load() const;
    void save(int score) const;

private:
    std::filesystem::path file_path_;
};

}  // namespace puyopuyo