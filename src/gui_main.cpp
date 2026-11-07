#include "storage/high_score_store.hpp"
#include "domain/game.hpp"

#include "raylib.h"
#include <random>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <optional>
#include <vector>
#include <algorithm>
#include <cmath>

enum class Screen {
    Title,
    HighScore,
    Tutorial,
    Playing,
    GameOver,
    Result
};

enum class PlayPhase {
    Falling,
    Gravity,
    Erasing
};

Color to_draw_color(puyopuyo::Color color) {
    switch (color) {
    case puyopuyo::Color::Red:    return RED;
    case puyopuyo::Color::Blue:   return BLUE;
    case puyopuyo::Color::Green:  return GREEN;
    case puyopuyo::Color::Yellow: return YELLOW;
    default:                    return BLANK;
    }
}

puyopuyo::Color random_color(std::mt19937& engine) {
    std::uniform_int_distribution<int> distribution(
        static_cast<int>(puyopuyo::Color::Red),
        static_cast<int>(puyopuyo::Color::Yellow)
    );

    return static_cast<puyopuyo::Color>(distribution(engine));
}

puyopuyo::Piece make_random_piece(int board_cols, std::mt19937& engine) {
    return {
        {0, (board_cols - 2) / 2},
        random_color(engine),
        random_color(engine),
        puyopuyo::Direction::Right
    };
}

int main() {
    InitWindow(800, 640, "Falling Puzzle Game");

    if (!IsWindowReady()) {
        return 1;
    }

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    InitAudioDevice();
    const bool audio_ready = IsAudioDeviceReady();

    Sound land_sound{};
    bool land_sound_ready = false;

    Sound menu_sound{};
    bool menu_sound_ready = false;

    Sound confirm_sound{};
    Sound game_over_sound{};
    bool confirm_sound_ready = false;
    bool game_over_sound_ready = false;

    Sound erase_sound{};
    bool erase_sound_ready = false;

    if (audio_ready) {
        const int sample_rate = 44100;
        const int sample_count = 11025;
        std::vector<float> samples(sample_count);

        for (int i = 0; i < sample_count; ++i) {
            const float time = static_cast<float>(i) / sample_rate;
            const float envelope =
                1.0f - static_cast<float>(i) / sample_count;

            samples[i] =
                0.15f * envelope
                * std::sin(2.0f * 3.14159265f * 440.0f * time);
        }

        Wave wave{};
        wave.frameCount = static_cast<unsigned int>(samples.size());
        wave.sampleRate = sample_rate;
        wave.sampleSize = 32;
        wave.channels = 1;
        wave.data = samples.data();

        land_sound = LoadSoundFromWave(wave);
        land_sound_ready = IsSoundValid(land_sound);

        const int menu_sample_count = 2646;  // 0.06秒
        samples.resize(menu_sample_count);

        for (int i = 0; i < menu_sample_count; ++i) {
            const float time = static_cast<float>(i) / sample_rate;
            const float attack = std::min(time / 0.003f, 1.0f);
            const float decay =
                1.0f - static_cast<float>(i) / menu_sample_count;

            samples[i] =
                0.08f * attack * decay
                * std::sin(2.0f * 3.14159265f * 880.0f * time);
        }

        wave.frameCount = static_cast<unsigned int>(samples.size());
        wave.data = samples.data();

        menu_sound = LoadSoundFromWave(wave);
        menu_sound_ready = IsSoundValid(menu_sound);

        const int erase_sample_count = 11025;  // 0.25秒
        samples.resize(erase_sample_count);

        float phase = 0.0f;

        for (int i = 0; i < erase_sample_count; ++i) {
            const float time = static_cast<float>(i) / sample_rate;
            const bool first_note = time < 0.07f;

            const float frequency = first_note ? 660.0f : 880.0f;
            const float note_time = first_note ? time : time - 0.07f;
            const float note_duration = first_note ? 0.07f : 0.18f;

            const float attack = std::min(note_time / 0.004f, 1.0f);
            const float decay = std::max(
                1.0f - note_time / note_duration,
                0.0f
            );

            samples[i] = 0.15f * attack * decay * (
                std::sin(phase)
                + 0.2f * std::sin(2.0f * phase)
            );

            phase += 2.0f * 3.14159265f * frequency / sample_rate;
        }

        wave.frameCount = static_cast<unsigned int>(samples.size());
        wave.data = samples.data();

        erase_sound = LoadSoundFromWave(wave);
        erase_sound_ready = IsSoundValid(erase_sound);
        const auto make_tone_sequence = [&](const std::vector<float>& notes,
                                            float note_duration) {
            const int note_samples = static_cast<int>(sample_rate * note_duration);
            std::vector<float> sequence(note_samples * notes.size());
            float phase = 0.0f;

            for (std::size_t note = 0; note < notes.size(); ++note) {
                for (int i = 0; i < note_samples; ++i) {
                    const float time = static_cast<float>(i) / sample_rate;
                    const float attack = std::min(time / 0.004f, 1.0f);
                    const float decay = 1.0f - static_cast<float>(i) / note_samples;
                    sequence[note * note_samples + i] =
                        0.12f * attack * decay *
                        (std::sin(phase) + 0.15f * std::sin(2.0f * phase));
                    phase += 2.0f * 3.14159265f * notes[note] / sample_rate;
                }
            }

            Wave sequence_wave{};
            sequence_wave.frameCount = static_cast<unsigned int>(sequence.size());
            sequence_wave.sampleRate = sample_rate;
            sequence_wave.sampleSize = 32;
            sequence_wave.channels = 1;
            sequence_wave.data = sequence.data();
            return LoadSoundFromWave(sequence_wave);
        };

        confirm_sound = make_tone_sequence({660.0f, 990.0f}, 0.06f);
        confirm_sound_ready = IsSoundValid(confirm_sound);
        game_over_sound = make_tone_sequence({330.0f, 261.625f, 196.0f}, 0.18f);
        game_over_sound_ready = IsSoundValid(game_over_sound);
    }

    const char* title = "FALLING PUZZLE GAME";
    const int title_font_size = 40;
    const int title_x =
        (GetScreenWidth() - MeasureText(title, title_font_size)) / 2;

    const char* menu_items[] = {
        "Start",
        "Quit"
    };
    const int menu_count = 2;
    const int menu_font_size = 28;

    const char* hint = "Press Esc to return to title";
    const int hint_size = 24;
    const int hint_x =
        (GetScreenWidth() - MeasureText(hint, hint_size)) / 2;

    int selected_index = 0;
    bool should_quit = false;

    Screen screen = Screen::Title;
    bool show_back_hint = false;

    const int board_rows = 12;
    const int board_cols = 6;
    const int cell_size = 40;
    const int board_x = 100;
    const int board_y = 80;
    const float radius = cell_size * 0.4f;

    puyopuyo::Game game(board_rows, board_cols);
    const float fall_interval = 0.36f;
    float fall_elapsed = 0.0f;

    std::random_device seed;
    std::mt19937 engine(seed());

    puyopuyo::Piece next_piece =
        make_random_piece(board_cols, engine);

    PlayPhase play_phase = PlayPhase::Falling;

    const float gravity_interval = 0.1f;
    float gravity_elapsed = 0.0f;

    const float erase_blink_interval = 0.12f;
    const int erase_blink_count = 4;
    const float erase_duration =
        erase_blink_interval * 2 * erase_blink_count;
    float erase_elapsed = 0.0f;

    std::vector<std::vector<puyopuyo::Position>> erase_groups;

    const float game_over_duration = 0.8f;
    float game_over_elapsed = 0.0f;

    const char* game_over_text = "GAME OVER";
    const int game_over_font_size = 48;
    const int game_over_x =
        (GetScreenWidth()
        - MeasureText(game_over_text, game_over_font_size)) / 2;
    const int game_over_y =
        (GetScreenHeight() - game_over_font_size) / 2;

    const char* result_items[] = {"Retry", "Title"};
    const int result_count = 2;
    int result_selected = 0;

    int high_score = 0;
    bool score_storage_error = false;

    std::optional<puyopuyo::HighScoreStore> high_score_store;

    bool confirm_exit = false;
    bool return_to_title = false;

    const char* exit_items[] = {"Yes", "No"};
    const int exit_count = 2;
    int exit_selected = 1;

    try {
        const char* local_app_data = std::getenv("LOCALAPPDATA");

        if (local_app_data != nullptr && local_app_data[0] != '\0') {
            const auto score_path =
                std::filesystem::path(local_app_data)
                / "FallingPuzzleGame"
                / "high_score.txt";

            high_score_store.emplace(score_path);
            high_score = high_score_store->load();
        } else {
            score_storage_error = true;
        }
    } catch (const std::exception& error) {
        score_storage_error = true;
        high_score_store.reset();
        TraceLog(LOG_WARNING, "High score load failed: %s", error.what());
    }

    bool new_high_score = false;

    const auto start_game = [&]() {
        if (game_over_sound_ready) {
            StopSound(game_over_sound);
        }
        game = puyopuyo::Game(board_rows, board_cols);

        game.try_spawn(make_random_piece(board_cols, engine));
        next_piece = make_random_piece(board_cols, engine);

        play_phase = PlayPhase::Falling;
        fall_elapsed = 0.0f;
        gravity_elapsed = 0.0f;
        game_over_elapsed = 0.0f;
        erase_elapsed = 0.0f;
        erase_groups.clear();

        new_high_score = false;

        screen = Screen::Playing;
    };

    while (!should_quit) {
        const bool close_requested = WindowShouldClose();

        const bool escape_requested =
            !confirm_exit
            && screen == Screen::Playing
            && IsKeyPressed(KEY_ESCAPE);

        bool opened_exit_this_frame = false;

        if (close_requested || escape_requested) {
            if (screen == Screen::Playing) {
                if (!confirm_exit) {
                    exit_selected = 1;
                }

                return_to_title = !close_requested;
                confirm_exit = true;
                opened_exit_this_frame = true;
            } else {
                should_quit = true;
            }
        }

        if (should_quit) {
            break;
        }

        if (confirm_exit) {
            if (!opened_exit_this_frame) {
                if (IsKeyPressed(KEY_ESCAPE)) {
                    confirm_exit = false;
                } else {
                    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                        exit_selected = (exit_selected + 1) % exit_count;
                        if (menu_sound_ready) {
                            PlaySound(menu_sound);
                        }
                    }

                    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                        exit_selected =
                            (exit_selected + exit_count - 1) % exit_count;
                            if (menu_sound_ready) {
                                PlaySound(menu_sound);
                            }
                    }

                    if (IsKeyPressed(KEY_ENTER)) {

                        if (confirm_sound_ready) {

                            PlaySound(confirm_sound);

                        }
                        if (exit_selected == 0) {
                            if (return_to_title) {
                                screen = Screen::Title;
                                selected_index = 0;
                                show_back_hint = false;
                                confirm_exit = false;
                            } else {
                                should_quit = true;
                            }
                        } else {
                            confirm_exit = false;
                        }
                    }
                }
            }

            while (GetKeyPressed() != 0) {
            }

        } else if (screen == Screen::Title) {
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                selected_index = (selected_index + 1) % menu_count;
                if (menu_sound_ready) {
                    PlaySound(menu_sound);
                }
            }

            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                selected_index =
                    (selected_index + menu_count - 1) % menu_count;
                    if (menu_sound_ready) {
                        PlaySound(menu_sound);
                    }
            }

            if (IsKeyPressed(KEY_ENTER)) {

                if (confirm_sound_ready) {

                    PlaySound(confirm_sound);

                }
                switch (selected_index) {
                case 0:
                    start_game();
                    break;

                case 1:
                    should_quit = true;
                    break;
                }

                show_back_hint = false;

                // 決定に使ったキーを次の画面へ持ち越さない
                while (GetKeyPressed() != 0) {
                }
            }
        } else if (screen == Screen::HighScore || screen == Screen::Tutorial) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                screen = Screen::Title;
                show_back_hint = false;
            } else {
                while (GetKeyPressed() != 0) {
                show_back_hint = true;
            }
            }
        } else if (screen == Screen::Playing && !game.is_game_over()) {
            const float delta_time = GetFrameTime();

            if (play_phase == PlayPhase::Falling) {
                fall_elapsed += delta_time;

                if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                    game.try_move(0, -1);
                }

                if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                    game.try_move(0, 1);
                }

                if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_Z)) {
                    game.try_rotate_clockwise();
                }

                const bool manual_fall = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);

                if (manual_fall || fall_elapsed >= fall_interval) {
                    const auto result = game.fall_one_row();
                    fall_elapsed = 0.0f;

                    if (result == puyopuyo::FallResult::Locked) {
                        if (land_sound_ready) {
                            PlaySound(land_sound);
                        }

                        play_phase = PlayPhase::Gravity;
                        gravity_elapsed = 0.0f;
                    }
                }
            } else if (play_phase == PlayPhase::Gravity) {
                gravity_elapsed += delta_time;

                if (gravity_elapsed >= gravity_interval) {
                    gravity_elapsed = 0.0f;

                    if (!game.apply_gravity_one_row()) {
                        erase_groups = game.find_erasable_groups();

                        if (!erase_groups.empty()) {
                            play_phase = PlayPhase::Erasing;
                            erase_elapsed = 0.0f;

                        } else {
                            // 空の対象を渡し、連鎖数をリセットする
                            (void)game.erase_once(erase_groups);

                            if (game.try_spawn(next_piece)) {
                                next_piece = make_random_piece(board_cols, engine);
                                play_phase = PlayPhase::Falling;
                                fall_elapsed = 0.0f;
                            } else {
                                if (game_over_sound_ready) {
                                    PlaySound(game_over_sound);
                                }
                                screen = Screen::GameOver;
                                game_over_elapsed = 0.0f;

                                while (GetKeyPressed() != 0) {
                                }
                            }
                        }
                    }
                }
            } else if (play_phase == PlayPhase::Erasing) {
                erase_elapsed += delta_time;

                if (erase_elapsed >= erase_duration) {
                    (void)game.erase_once(erase_groups);

                    if (erase_sound_ready) {
                        const int pitch_steps =
                            std::min(game.current_chain() - 1, 4) * 3;

                        const float pitch = std::pow(2.0f, pitch_steps / 12.0f);

                        SetSoundPitch(erase_sound, pitch);
                        PlaySound(erase_sound);
                    }

                    erase_groups.clear();
                    erase_elapsed = 0.0f;

                    play_phase = PlayPhase::Gravity;
                    gravity_elapsed = 0.0f;
                }
            }
        } else if (screen == Screen::GameOver) {
            bool key_pressed = false;

            while (GetKeyPressed() != 0) {
                key_pressed = true;
            }
            game_over_elapsed += GetFrameTime();

            if (game_over_elapsed > game_over_duration) {
                game_over_elapsed = game_over_duration;
            }

            if (key_pressed) {
                result_selected = 0;
                screen = Screen::Result;
            }
        } else if (screen == Screen::Result) {
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                result_selected = (result_selected + 1) % result_count;
                if (menu_sound_ready) {
                    PlaySound(menu_sound);
                }
            }

            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                result_selected =
                    (result_selected + result_count - 1) % result_count;
                    if (menu_sound_ready) {
                        PlaySound(menu_sound);
                    }
            }

            if (IsKeyPressed(KEY_ENTER)) {

                if (confirm_sound_ready) {

                    PlaySound(confirm_sound);

                }
                if (result_selected == 0) {
                    start_game();
                } else {
                    screen = Screen::Title;
                    selected_index = 0;
                    show_back_hint = false;
                }

                while (GetKeyPressed() != 0) {
                }
            }
        }

        if (game.score() > high_score) {
            high_score = game.score();
            new_high_score = true;

            if (high_score_store.has_value()) {
                try {
                    high_score_store->save(high_score);
                } catch (const std::exception& error) {
                    score_storage_error = true;
                    TraceLog(
                        LOG_WARNING,
                        "High score save failed: %s",
                        error.what()
                    );
                }
            }
        }

        BeginDrawing();

        ClearBackground(Color{24, 28, 38, 255});

        if (screen == Screen::Title) {
            DrawText(title, title_x, 120, title_font_size, RAYWHITE);

        const char* high_score_text =
            TextFormat("High Score: %i", high_score);

        DrawText(
            high_score_text,
            (GetScreenWidth() - MeasureText(high_score_text, 24)) / 2,
            200, 24, RAYWHITE
        );

            for (int i = 0; i < menu_count; ++i) {
                const int item_width =
                    MeasureText(menu_items[i], menu_font_size);
                const int item_x =
                    (GetScreenWidth() - item_width) / 2;
                const int item_y = 260 + i * 60;

                const Color item_color =
                    (i == selected_index) ? YELLOW : RAYWHITE;

                DrawText(
                    menu_items[i], item_x, item_y,
                    menu_font_size, item_color
                );
            }
        } else if (screen == Screen::Playing || screen == Screen::GameOver) {
            const auto& board = game.board();

            for (int row = 0; row < board.rows(); ++row) {
                for (int col = 0; col < board.cols(); ++col) {
                    const int x = board_x + col * cell_size;
                    const int y = board_y + row * cell_size;

                    DrawRectangleLines(x, y, cell_size, cell_size, GRAY);

                    const auto color = board.at({row, col});

                    if (color != puyopuyo::Color::None) {
                        bool hide_puyo = false;

                        if (play_phase == PlayPhase::Erasing) {
                            const int blink_step =
                                static_cast<int>(erase_elapsed / erase_blink_interval);

                            if (blink_step % 2 == 1) {
                                for (const auto& group : erase_groups) {
                                    for (const auto& position : group) {
                                        if (position.row == row && position.col == col) {
                                            hide_puyo = true;
                                            break;
                                        }
                                    }

                                    if (hide_puyo) {
                                        break;
                                    }
                                }
                            }
                        }

                        if (!hide_puyo) {
                            DrawCircle(
                                x + cell_size / 2, y + cell_size / 2,
                                radius, to_draw_color(color)
                            );
                        }
                    }
                }
            }

            if (game.active_piece().has_value()) {
                const auto& piece = *game.active_piece();
                const auto satellite = puyopuyo::satellite_position(piece);

                const int pivot_x =
                    board_x + piece.pivot_position.col * cell_size
                    + cell_size / 2;
                const int pivot_y =
                    board_y + piece.pivot_position.row * cell_size
                    + cell_size / 2;

                const int satellite_x =
                    board_x + satellite.col * cell_size
                    + cell_size / 2;
                const int satellite_y =
                    board_y + satellite.row * cell_size
                    + cell_size / 2;

                DrawCircle(
                    pivot_x, pivot_y, radius,
                    to_draw_color(piece.pivot_color)
                );
                DrawCircle(
                    satellite_x, satellite_y, radius,
                    to_draw_color(piece.satellite_color)
                );
            }

            if (play_phase == PlayPhase::Erasing && !erase_groups.empty()) {
                const int chain = game.current_chain() + 1;

                if (chain >= 2) {
                    std::size_t selected = 0;
                    int selected_top = board_rows;

                    for (std::size_t i = 0; i < erase_groups.size(); ++i) {
                        int top = board_rows;

                        for (const auto& position : erase_groups[i]) {
                            top = std::min(top, position.row);
                        }

                        if (i == 0
                            || erase_groups[i].size() > erase_groups[selected].size()
                            || (erase_groups[i].size() == erase_groups[selected].size()
                                && top < selected_top)) {
                            selected = i;
                            selected_top = top;
                        }
                    }

                    int left = board_cols;
                    int right = 0;

                    for (const auto& position : erase_groups[selected]) {
                        left = std::min(left, position.col);
                        right = std::max(right, position.col);
                    }

                    const char* chain_text = TextFormat("%i CHAIN!", chain);
                    const int font_size = 32;
                    const int text_width = MeasureText(chain_text, font_size);

                    const int center_x =
                        board_x + (left + right + 1) * cell_size / 2;

                    const int text_x = std::clamp(
                        center_x - text_width / 2,
                        8,
                        GetScreenWidth() - text_width - 8
                    );

                    const int text_y = std::max(
                        8,
                        board_y + selected_top * cell_size - font_size - 8
                    );

                    DrawText(
                        chain_text,
                        text_x + 2, text_y + 2,
                        font_size, BLACK
                    );

                    DrawText(
                        chain_text,
                        text_x, text_y,
                        font_size, MAGENTA
                    );
                }
            }

            DrawText("Next", 420, 80, 28, RAYWHITE);

            DrawText(
                TextFormat("Score: %i", game.score()),
                420, 220,
                28, RAYWHITE
            );

            DrawText(
                TextFormat("High Score: %i", high_score),
                420, 270, 24, RAYWHITE
            );

            DrawCircle(
                440, 140, radius,
                to_draw_color(next_piece.pivot_color)
            );
            DrawCircle(
                440 + cell_size, 140, radius,
                to_draw_color(next_piece.satellite_color)
            );
        } else if (screen == Screen::Result) {
            const char* heading = "RESULT";
            const int heading_size = 40;

            DrawText(
                heading,
                (GetScreenWidth() - MeasureText(heading, heading_size)) / 2,
                120, heading_size, RAYWHITE
            );

            const char* score_text = TextFormat("Score: %i", game.score());

            DrawText(
                score_text,
                (GetScreenWidth() - MeasureText(score_text, 28)) / 2,
                230, 28, RAYWHITE
            );

            const char* high_score_text =
                TextFormat("High Score: %i", high_score);

            DrawText(
                high_score_text,
                (GetScreenWidth() - MeasureText(high_score_text, 24)) / 2,
                280, 24, RAYWHITE
            );

            for (int i = 0; i < result_count; ++i) {
                const int item_x =
                    (GetScreenWidth()
                    - MeasureText(result_items[i], menu_font_size)) / 2;

                DrawText(
                    result_items[i],
                    item_x, 340 + i * 60,
                    menu_font_size,
                    i == result_selected ? YELLOW : RAYWHITE
                );
            }

            if (new_high_score) {
                const char* message = "NEW HIGH SCORE!";
                const int message_size = 24;
                const int message_x =
                    (GetScreenWidth() - MeasureText(message, message_size)) / 2;

                DrawText(message, message_x, 180, message_size, YELLOW);
            }
        } else {
            if (show_back_hint) {
                DrawText(hint, hint_x, 500, hint_size, RAYWHITE);
            }
        }

        if (screen == Screen::GameOver) {
            const float progress =
                game_over_elapsed / game_over_duration;

            DrawRectangle(
                0, 0,
                GetScreenWidth(), GetScreenHeight(),
                Fade(BLACK, 0.65f * progress)
            );

            DrawText(
                game_over_text,
                game_over_x, game_over_y,
                game_over_font_size,
                Fade(RAYWHITE, progress)
            );
        }

        if (confirm_exit) {
            DrawRectangle(
                0, 0,
                GetScreenWidth(), GetScreenHeight(),
                Fade(BLACK, 0.75f)
            );

            const char* question =
                return_to_title ? "Return to title?" : "Quit the game?";

            DrawText(
                question,
                (GetScreenWidth() - MeasureText(question, 32)) / 2,
                240, 32, RAYWHITE
            );

            for (int i = 0; i < exit_count; ++i) {
                const int item_x =
                    (GetScreenWidth() - MeasureText(exit_items[i], 28)) / 2;

                DrawText(
                    exit_items[i],
                    item_x, 320 + i * 60,
                    28,
                    i == exit_selected ? YELLOW : RAYWHITE
                );
            }
        }

        if (score_storage_error) {
            DrawText(
                "High score storage unavailable",
                20, GetScreenHeight() - 30,
                20, ORANGE
            );
        }

        EndDrawing();
    }

    if (land_sound_ready) {
        UnloadSound(land_sound);
    }

    if (menu_sound_ready) {
        UnloadSound(menu_sound);
    }

    if (erase_sound_ready) {
        UnloadSound(erase_sound);
    }

    if (confirm_sound_ready) {
        UnloadSound(confirm_sound);
    }
    if (game_over_sound_ready) {
        UnloadSound(game_over_sound);
    }

    if (audio_ready) {
        CloseAudioDevice();
    }

    CloseWindow();
    return 0;
}
