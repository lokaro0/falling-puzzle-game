#include "domain/board.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

using puyopuyo::Board;
using puyopuyo::Color;
using puyopuyo::Position;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Function>
void require_out_of_range(Function operation) {
    try {
        operation();
    } catch (const std::out_of_range&) {
        return;
    }

    throw std::runtime_error("std::out_of_range was expected.");
}

template <typename Function>
void require_invalid_argument(Function operation) {
    try {
        operation();
    } catch (const std::invalid_argument&) {
        return;
    }

    throw std::runtime_error("std::invalid_argument was expected.");
}

void test_new_board_is_empty() {
    const Board board(2, 3);

    require(board.rows() == 2, "Row count is incorrect.");
    require(board.cols() == 3, "Column count is incorrect.");

    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            require(board.at(Position{row, col}) == Color::None, "A new cell must be empty.");
        }
    }
}

void test_cell_can_be_written_and_copied() {
    Board board(2, 3);
    board.at(Position{1, 2}) = Color::Red;

    const Board copied_board = board;

    require(copied_board.at(Position{1, 2}) == Color::Red, "Copied cell color is incorrect.");

    board.at(Position{1, 2}) = Color::Blue;
    require(
        copied_board.at(Position{1, 2}) == Color::Red,
        "Copied board must own independent cell data.");
}

void test_clear_removes_all_colors() {
    Board board(2, 3);
    board.at(Position{0, 0}) = Color::Green;
    board.at(Position{1, 2}) = Color::Yellow;

    board.clear();

    require(board.at(Position{0, 0}) == Color::None, "clear() did not reset the first cell.");
    require(board.at(Position{1, 2}) == Color::None, "clear() did not reset the last cell.");
}

void test_invalid_coordinates_are_rejected() {
    Board board(2, 3);

    require(board.contains(Position{1, 2}), "Valid coordinate was rejected.");
    require(!board.contains(Position{-1, 0}), "Negative row was accepted.");
    require(!board.contains(Position{2, 0}), "Row beyond board was accepted.");
    require(!board.contains(Position{0, 3}), "Column beyond board was accepted.");

    require_out_of_range([&board] { board.at(Position{-1, 0}); });
    require_out_of_range([&board] { board.at(Position{2, 0}); });
    require_out_of_range([&board] { board.at(Position{0, 3}); });

    require(
        board.contains(Position{1, 2}),
        "Valid Position was rejected.");

    require_out_of_range([&board] {
        board.at(Position{-1, 0});
});
}

void test_invalid_dimensions_are_rejected() {
    require_invalid_argument([] { Board(0, 6); });
    require_invalid_argument([] { Board(12, -1); });
}

}  // namespace

int main() {
    try {
        test_new_board_is_empty();
        test_cell_can_be_written_and_copied();
        test_clear_removes_all_colors();
        test_invalid_coordinates_are_rejected();
        test_invalid_dimensions_are_rejected();
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All Board tests passed.\n";
    return 0;
}