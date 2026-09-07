#include "storage/high_score_store.hpp"

#include <fstream>
#include <stdexcept>
#include <utility>

namespace puyopuyo {

HighScoreStore::HighScoreStore(std::filesystem::path file_path)
    : file_path_(std::move(file_path)) {
}

int HighScoreStore::load() const {
    if (!std::filesystem::exists(file_path_)) {
        return 0;
    }

    std::ifstream input(file_path_);

    if (!input) {
        throw std::runtime_error("Failed to open high-score file.");
    }

    int score = 0;

    if (!(input >> score) || score < 0) {
        return 0;
    }

    return score;
}

void HighScoreStore::save(int score) const {
    if (score < 0) {
        throw std::invalid_argument("High score cannot be negative.");
    }

    const std::filesystem::path parent_directory = file_path_.parent_path();

    if (!parent_directory.empty()) {
        std::filesystem::create_directories(parent_directory);
    }

    std::ofstream output(file_path_);

    if (!output) {
        throw std::runtime_error("Failed to write high-score file.");
    }

    output << score << '\n';

    if (!output) {
        throw std::runtime_error("Failed to save high score.");
    }
}

}  // namespace puyopuyo