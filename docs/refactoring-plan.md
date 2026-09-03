# リファクタリング計画

## 目的

C++ / ncurses で制作した落下パズルゲームを、公開可能な就活ポートフォリオとして改善する。

初版の動作を維持しながら、ゲームロジック・入力／描画・ハイスコア保存を分離する。
また、ゲーム規則を単体テストで検証できる構成にし、設計判断と工夫をREADMEで説明できる状態を目指す。

## 作業方針

- 動作する初版は `main` ブランチに保持する。
- 改善作業は `refactor/separate-game-logic` ブランチで進める。
- 変更は小さな単位で行い、各段階でビルド・テスト・必要な手動確認をする。
- 改善版の盤面データには `std::vector` を利用する。独自 `vector` / `matrix` は初版の `main` ブランチとGit履歴に学習成果として残す。
- ncursesに依存するコードをゲーム規則から排除し、画面なしでテストできるようにする。

## 現在の状態

### 完了済み

- `refactor/separate-game-logic` ブランチを作成した。
- WSL / Ubuntu 上で ncurses を使う初版のビルド・実行を確認した。
- `Board` クラスを追加した。
  - 盤面のサイズ、セル参照、範囲判定、初期化を担当する。
  - セルの所有には `std::vector<Color>` を使う。
- `Color` をゲーム全体で使う共通型として分離した。
- `Position` を盤面座標の型として追加した。
- `Piece` を追加した。
  - 操作中の2個組を、中心ぷよ・相方ぷよ・相方の方向として表す。
- `Game` を追加した。
  - 生成、移動、回転、着地、固定、ゲームオーバー判定を担当する。
  - 4個以上の同色グループ探索、消去、重力による落下、得点計算を担当する。
- CMakeを、ゲーム本体・ドメインライブラリ・テスト実行ファイルを扱える構成へ変更した。
- CTestで `Board`、`Piece`、`Game` の単体テストを実行し、成功を確認した。
  - 盤面初期化、コピー、範囲外アクセス
  - 相方座標、移動、回転
  - 壁・固定ぷよとの衝突
  - 着地と固定
  - 4個消去、3個非消去、複数グループの同時消去
  - 消去後の落下で発生する追加消去

### チェイン数の仕様

このゲームの `chain_count` は、一般的なぷよぷよの連鎖段数ではなく、元コードに合わせて「一連の解決処理で消えたグループ数」を表す。

```text
赤4個と青4個が同時に消える
→ chain_count = 2

その後の落下で黄4個が消える
→ chain_count = 3
```

### 現在の主な構成

```text
src/
├── puyo.cpp
└── domain/
    ├── board.hpp
    ├── board.cpp
    ├── color.hpp
    ├── game.hpp
    ├── game.cpp
    ├── piece.hpp
    ├── piece.cpp
    └── position.hpp

tests/
├── board_test.cpp
├── game_test.cpp
└── piece_test.cpp
```

現時点では、`domain/` の新しいゲームロジックはテスト可能な状態になっている。一方、ncursesを使う `src/puyo.cpp` はまだ初版の `PuyoArray` を使っており、新しい `Game` には接続していない。

## 目標アーキテクチャ

main.cpp
 ├─ ui/
 │   ├─ NcursesSession    # ncursesの初期化・終了
 │   ├─ NcursesInput      # キー入力をCommandへ変換
 │   └─ NcursesRenderer   # Gameの状態を描画
 │
 ├─ domain/
 │   ├─ Color             # ぷよの色
 │   ├─ Position          # 盤面座標
 │   ├─ Board             # 固定ぷよの盤面
 │   ├─ Piece             # 落下中の2個組
 │   └─ Game              # 移動・回転・消去・連鎖・得点
 │
 └─ storage/
     └─ HighScoreRepository  # ハイスコアの読み書き

依存関係は次の方向だけにする。

ui ────────→ domain ←──────── storage
main ──────→ ui / domain / storage
tests ─────→ domain

`domain` は ncurses・ファイルI/O・待機処理を参照しない。

## 段階別の作業

### 1. ドメインモデルの基盤を整える（完了）

対象:

- `src/domain/color.hpp`
- `src/domain/position.hpp`
- `src/domain/piece.hpp`
- `src/domain/board.hpp`
- `src/domain/board.cpp`

内容:

- `Color` を共通型として定義する。
- `Position` で行・列の座標を表す。
- `Piece` で操作中の2個のぷよ、位置、向きを表す。
- `Board` は固定済みぷよの保存だけを担当する。

完了条件:

- `Board` と `Piece` がncursesなしでビルドできる。
- 盤面初期化、座標範囲、コピー、色の設定をテストできる。

### 2. Gameへゲーム規則を移す（完了）

対象:

- `src/domain/game.hpp`
- `src/domain/game.cpp`

内容:

- 新しい2個組の生成
- 左右移動
- 下移動
- 回転
- 壁・固定ぷよとの衝突判定
- 着地と固定
- 重力による落下
- 4個以上の同色グループの探索・消去
- 連鎖、得点、ゲームオーバー

完了条件:

- `Game` がncursesなしでゲームを進行できる。
- `Game` は描画関数、`usleep`、ファイル操作を呼ばない。
- 元のゲームと同等の基本操作・消去・連鎖ができる。

### 3. ゲーム規則のテストを追加する（完了）

対象:

- `tests/board_test.cpp`
- `tests/game_test.cpp`

内容:

- 壁際の移動拒否
- 固定ぷよとの衝突
- 回転成功・回転失敗
- 着地と固定
- 重力による落下
- 4個消去、3個非消去、異色非消去
- 複数グループの同時消去
- 連鎖
- 得点
- ゲームオーバー

完了条件:

cmake --build build/ubuntu --parallel
ctest --test-dir build/ubuntu --output-on-failure

が成功する。

### 4. ncursesの入力・描画を分離する（次の作業）

対象:

- `src/ui/ncurses_session.hpp`
- `src/ui/ncurses_session.cpp`
- `src/ui/ncurses_input.hpp`
- `src/ui/ncurses_input.cpp`
- `src/ui/ncurses_renderer.hpp`
- `src/ui/ncurses_renderer.cpp`

内容:

- ncursesの初期化と終了をRAIIで扱う。
- キー入力をゲーム用の命令へ変換する。
- `Game` の状態から盤面、スコア、連鎖、次のぷよを描画する。
- 点滅・待機をUI側で扱う。

完了条件:

- `domain/` に `<curses.h>` と `unistd.h` が含まれない。
- 例外・早期終了時も `endwin()` が呼ばれる。

### 5. ハイスコア保存を分離する

対象:

- `src/storage/high_score_repository.hpp`
- `src/storage/high_score_repository.cpp`

内容:

- ハイスコアの読み込み
- ハイスコアの保存
- ファイルがない場合の初期値
- 読み書き失敗時の扱い

完了条件:

- `Game` がファイルを直接扱わない。
- ハイスコア保存先をゲーム規則から変更できる。

### 6. mainを組み立て専用にする

対象:

- `src/main.cpp`

内容:

- UI、Game、ハイスコア保存の生成
- 入力、更新、描画のゲームループ
- 終了処理

完了条件:

- 現在の `src/puyo.cpp` を廃止または `src/main.cpp` へ置換する。
- `main.cpp` はゲーム規則の詳細を持たない。

### 7. 公開用ドキュメントを整える

対象:

- `README.md`
- `docs/architecture.md`
- `docs/refactoring-plan.md`
- `assets/`

内容:

- ゲーム画面のスクリーンショットまたはGIF
- 概要、操作方法、動作環境
- WSLでのビルド・実行・テスト手順
- ディレクトリ構成
- アーキテクチャ図
- 工夫した点と設計判断
- 今後の改善案

完了条件:

- 初見の人がREADMEだけでビルド・実行・テストできる。
- 就活で説明する設計上の意図が文章と図で伝わる。

## 品質確認

各段階で次を行う。

cmake -S . -B build/ubuntu
cmake --build build/ubuntu --parallel
ctest --test-dir build/ubuntu --output-on-failure
./build/ubuntu/puyopuyo
git status --short --branch

確認内容:

- コンパイル警告が増えていないか
- 単体テストが通るか
- 実際のゲームが操作できるか
- 意図しないファイルをGit管理していないか

## 公開前チェックリスト

- [ ] `main` に初版が保持されている
- [ ] 改善版の全テストが成功する
- [ ] WSLでゲームを実行できる
- [ ] READMEの手順で初見の環境からビルドできる
- [ ] ゲーム画面の画像またはGIFがある
- [ ] 設計図と工夫した点を説明している
- [ ] ビルド生成物、ハイスコア、個人情報を含めていない
- [ ] GitHubで公開して内容を確認した

## 公開後の改善候補

公開版を完成させた後、時間と目的に応じて検討する。

- スタート画面、ゲームオーバー画面、リスタート操作
- 消去時の点滅などのUI演出
- 難易度設定
- 得点ルールの拡張
- 効果音
