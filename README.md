# Falling Puzzle Game

C++とncursesで制作した、ターミナル上で動作する落下パズルゲームです。

## Features

- ランダムな2色ピースの生成
- 左右移動・落下・回転
- 着地判定と盤面への固定
- 4個以上つながった同色ピースの消去
- 連鎖、スコア、次ピースの表示
- ローカルのハイスコア保存

## Controls

| Key | Action |
| --- | --- |
| ← / → | Move left / right |
| ↓ | Move down |
| Z | Rotate |
| Q | Quit |

## Tech Stack

- C++
- ncurses
- Custom `vector` / `matrix` implementation
- Doxygen comments

## Project Structure

```text
src/
└─ puyo.cpp

## Build and Run (WSL / Ubuntu)

This project requires a Linux environment because it uses `ncurses`.

```bash
sudo apt update
sudo apt install -y build-essential cmake libncurses-dev

cmake -S . -B build
cmake --build build
./build/falling-puzzle