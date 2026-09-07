#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "storage/high_score_store.hpp"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class TemporaryDirectory final {
public:
    TemporaryDirectory() {
        const auto unique_number =
            std::chrono::steady_clock::now().time_since_epoch().count();

        path_ = std::filesystem::temp_directory_path()
            / ("puyopuyo_high_score_test_" + std::to_string(unique_number));

        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_);
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

private:
    std::filesystem::path path_;
};

void test_load_returns_zero_when_file_is_missing() {
    const TemporaryDirectory directory;
    const puyopuyo::HighScoreStore store(directory.path() / "high_score.txt");

    require(
        store.load() == 0,
        "A missing high-score file must be treated as score 0."
    );
}

void test_save_then_load_returns_saved_score() {
    const TemporaryDirectory directory;

    const puyopuyo::HighScoreStore store(
        directory.path() / "data" / "high_score.txt"
    );

    store.save(1200);

    require(
        store.load() == 1200,
        "A saved high score must be loaded correctly."
    );
}

void test_load_returns_zero_for_invalid_file_contents() {
    const TemporaryDirectory directory;
    const std::filesystem::path file_path = directory.path() / "high_score.txt";

    std::ofstream output(file_path);
    output << "invalid score\n";

    const puyopuyo::HighScoreStore store(file_path);

    require(
        store.load() == 0,
        "Invalid high-score contents must be treated as score 0."
    );
}

}  // namespace

int main() {
    try {
        test_load_returns_zero_when_file_is_missing();
        test_save_then_load_returns_saved_score();
        test_load_returns_zero_for_invalid_file_contents();
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}