# Falling Puzzle Game

[![CI](https://github.com/lokaro0/falling-puzzle-game/actions/workflows/ci.yml/badge.svg)](https://github.com/lokaro0/falling-puzzle-game/actions/workflows/ci.yml)

C++17とncursesで実装した、ターミナル上で動作する落ちものパズルゲームです。

動作する初版コードを分析し、ゲームロジック・画面表示・入力処理・ハイスコア保存を分離しました。ゲームロジックをncursesやファイルI/Oから独立させることで、自動テスト可能な構成に改善しています。

![ゲーム画面](assets/gameplay.png)

## 主な特徴

- ランダムな2個組ブロックの生成
- 次に出現する2個組ブロックの表示
- 中央が塞がれている場合の上端における出現位置探索
- 自然落下、左右移動、手動落下、時計回りの回転
- 同色のブロックが4個以上つながった場合の消去
- 消去時の点滅演出
- 消去後の段階的な重力表示と追加消去
- 片方だけが接地した場合の個別落下
- スコアとハイスコアの表示・保存
- ゲームオーバー判定
- CTestによるゲームロジックと保存処理の自動テスト
- GitHub ActionsによるUbuntu上での自動ビルド・テスト

## 使用技術

- C++17
- CMake 3.20以上
- ncurses
- CTest
- GitHub Actions
- GCC／Clang

## ビルドと実行

### 必要な環境

- C++17対応コンパイラ
- CMake 3.20以上
- ncurses
- LinuxまたはWSL

Ubuntu／WSLでは、必要なパッケージを次のコマンドでインストールできます。

```bash
sudo apt update
sudo apt install -y build-essential cmake libncurses-dev
```

リポジトリを取得し、プロジェクトのルートディレクトリでビルドします。

```bash
git clone https://github.com/lokaro0/falling-puzzle-game.git
cd falling-puzzle-game

cmake -S . -B build/ubuntu \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON

cmake --build build/ubuntu --parallel
./build/ubuntu/puyopuyo
```

## 操作方法

| キー | 操作 |
|---|---|
| `←` / `A` | 左へ移動 |
| `→` / `D` | 右へ移動 |
| `↓` / `S` | 手動で落下 |
| `R` / `Z` | 時計回りに回転 |
| `Q` | 終了 |

ハイスコアは実行時に`data/high_score.txt`へ保存されます。このファイルは個人のプレイ記録であるため、Gitの管理対象から除外しています。

## テスト

ビルド後、次のコマンドで全テストを実行できます。

```bash
ctest --test-dir build/ubuntu --output-on-failure
```

4つのテスト実行ファイルで、次の項目を確認しています。

- `Board`：盤面の生成、範囲判定、コピー、初期化
- `Piece`：2個組ブロックの移動、回転、相方の座標計算
- `Game`：着地、出現位置探索、消去、段階的な重力、追加消去、片方だけが接地した場合の落下
- `HighScoreStore`：ハイスコアの読み書きと異常入力

`Game::resolve()`が返す`erased_group_count`は、1回の解決処理で消去された連結グループの累計です。一般的な落ちものパズルにおける連鎖段数とは異なります。

GitHub Actionsでは、`main`へのpushとPull Requestごとに、Ubuntu 24.04上で設定、ビルド、テストを実行します。

## 設計

ソースコードを次の責務に分離しています。

```text
src/
├─ main.cpp                 # 各要素を組み合わせてゲーム進行を制御
├─ domain/                  # ncursesに依存しないゲームルール
│  ├─ board.*               # 盤面
│  ├─ color.hpp             # ブロックの色
│  ├─ position.hpp          # 盤面上の座標
│  ├─ piece.*               # 2個組ブロックの移動と回転
│  └─ game.*                # 落下、固定、消去、重力、スコア
├─ storage/
│  └─ high_score_store.*    # ハイスコアのファイル保存
└─ ui/
   ├─ ncurses_session.*     # ncursesの開始と終了
   ├─ ncurses_input.*       # キー入力の変換
   └─ ncurses_renderer.*    # 盤面と情報の描画
```

主な設計方針は次のとおりです。

- ゲーム規則をncursesから分離し、単体テストを可能にする
- 盤面のセル管理に`std::vector`を使用し、手動のメモリ管理を避ける
- ncursesの開始と終了をRAIIで管理する
- ハイスコアのファイルI/Oをゲーム規則から分離する
- `main`には入力、更新、描画を組み合わせる役割だけを持たせる

詳しい構成は[アーキテクチャ資料](docs/architecture.md)に記録しています。

## 初版を基準にした改善

初版の動作を維持しながら、段階的に次の改善を行いました。

- ゲームロジックとncurses処理の分離
- 入力処理と画面描画の分離
- ハイスコア保存処理の分離
- 手動メモリ管理から`std::vector`への移行
- ncursesセッションへのRAIIの導入
- 片方だけが接地した場合の重力処理の修正
- 初版の速度感を維持した落下間隔と、段階的な重力表示の追加
- 次ピース表示、消去点滅、上端での出現位置探索の復元
- 終了入力後に追加のゲーム更新が発生しないよう修正
- ゲームロジックと保存処理の自動テスト追加
- CMakeによるビルド手順の整備
- GitHub Actionsによる継続的インテグレーションの追加

改善内容と判断過程は[リファクタリング記録](docs/refactoring-plan.md)に記録しています。
