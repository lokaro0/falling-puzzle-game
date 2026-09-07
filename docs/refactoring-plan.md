# リファクタリング記録と公開準備計画

## 目的

C++ / ncurses で制作したぷよぷよ風の落下パズルゲームを、公開可能な就活ポートフォリオとして改善する。

初版の動作を `main` ブランチに保持したまま、ゲームルール・入力と描画・ハイスコア保存を分離した。現在は、分離後の構成を説明するドキュメントと画面素材を整え、公開できる状態にする段階である。

## 作業方針

- 動作する初版は `main` ブランチに保持する。
- 改善作業は `refactor/separate-game-logic` ブランチで行う。
- 変更は小さな単位で行い、各段階でビルド・自動テスト・必要な手動確認を行う。
- 盤面データには `std::vector` を利用し、手動メモリ管理を現行実装から除外する。
- `domain` 層は ncurses・ファイルI/O・待機処理に依存させず、画面なしでテストできるようにする。

## 完了した作業

### ドメイン層の分離

以下を `src/domain/` に分離した。

- `Color`：ぷよの色
- `Position`：盤面上の座標
- `Board`：固定済みぷよの盤面
- `Piece`：操作中の2個組ぷよ
- `Game`：移動、回転、固定、消去、重力、得点、ゲームオーバー判定

盤面のセル管理には `std::vector<Color>` を使用している。これにより、メモリ確保・解放・コピーを標準ライブラリへ任せ、ゲーム規則の実装に集中できるようにした。

### UI層の分離

以下を `src/ui/` に分離した。

- `NcursesSession`：ncurses の初期化と終了を RAII で管理
- `NcursesInput`：キー入力をゲーム操作へ変換
- `NcursesRenderer`：盤面、スコア、ハイスコア、ゲームオーバー表示

操作は矢印キーに加え、A / D / S、R / Z、Q に対応している。

### ハイスコア保存の分離

`src/storage/high_score_store.*` を追加した。

- ファイルが存在しない場合はスコア `0` として扱う
- 不正な内容はスコア `0` として扱う
- 新しいハイスコアを達成した時点で `data/high_score.txt` へ保存する
- 実行時のハイスコアファイルは Git 管理から除外する

### 重力処理の修正

2個組の片方だけが接地した場合、もう片方が空中で固定される問題を修正した。

```text
2個組を固定
↓
盤面全体へ重力を適用
↓
消去・連鎖処理
```

これにより、片方が障害物に接しても、もう片方は空いている列を下まで落下する。

### テスト

CTest で4つのテスト実行ファイルを実行できる。

- `puyopuyo_board_tests`
- `puyopuyo_piece_tests`
- `puyopuyo_game_tests`
- `puyopuyo_high_score_store_tests`

主な検証対象は以下のとおり。

- 盤面の初期化、コピー、範囲判定
- 2個組ぷよの移動、回転、相方座標
- 衝突、着地、固定、ゲームオーバー
- 4個消去、複数グループの同時消去、追加消去
- 片方だけが接地した場合の重力処理
- ハイスコアの読み込み、保存、不正なファイル内容

## 現在の構成

```text
src/
├─ main.cpp                 # ゲームループと各層の組み立て
├─ domain/
│  ├─ board.*
│  ├─ color.hpp
│  ├─ game.*
│  ├─ piece.*
│  └─ position.hpp
├─ storage/
│  └─ high_score_store.*
└─ ui/
   ├─ ncurses_input.*
   ├─ ncurses_renderer.*
   └─ ncurses_session.*

tests/
├─ board_test.cpp
├─ game_test.cpp
├─ high_score_store_test.cpp
└─ piece_test.cpp

docs/
└─ refactoring-plan.md
```

## 依存関係

```text
main ──→ domain
  ├────→ ui ──→ domain
  └────→ storage

tests ──→ domain
      └─→ storage
```

- `domain` は ncurses とファイルI/Oを参照しない
- `ui` は `Game` の状態を読むが、ゲーム規則を実装しない
- `storage` はハイスコアの読み書きだけを担当する
- `main` は各層を組み合わせ、入力・更新・描画の順序を制御する

## チェイン数の仕様

このゲームの `chain_count` は、一般的なぷよぷよの連鎖段数ではなく、初版に合わせて「一連の解決処理で消えたグループ数」を表す。

```text
赤4個と青4個が同時に消える
→ chain_count = 2

その後の落下で黄4個が消える
→ chain_count = 3
```

## 公開までに行うこと

1. README の内容と実際の構成・操作方法が一致しているか確認する  
2. `docs/architecture.md` を作成し、設計判断と依存関係を説明する  
3. ゲーム画面のスクリーンショットまたはGIFを `assets/` に追加する  
4. クリーンな環境でビルド・テスト・実行手順を確認する  
5. GitHub 公開前に、個人用ハイスコア・ビルド生成物・不要ファイルが含まれないことを確認する  
6. `refactor/separate-game-logic` の変更を確認し、公開用の `main` ブランチへ反映する