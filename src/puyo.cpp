/**
 * @file puyo.cpp
 * @brief ぷよぷよ風パズルゲームのメイン実装
 * @author
 * @date
 */

#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <random>
#include <fstream>

// ゲームの進行速度を調整するためのスリープに用いる値（マイクロ秒）
#define GMCLOCK 60000

/**
 * @enum puyocolor
 * @brief ぷよの色を表す列挙型
 */
enum puyocolor {
    NONE = 0, ///< ぷよ無し
    RED,      ///< 赤ぷよ
    BLUE,     ///< 青ぷよ
    GREEN,    ///< 緑ぷよ
    YELLOW    ///< 黄ぷよ
};

/**
 * @class vector
 * @brief 1次元配列の自作ベクタークラス
 * @tparam T 要素型
 */
template< class T >
class vector{
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    vector( ):num(0), x(NULL){}
    /**
     * @brief サイズ指定コンストラクタ
     * @param n 配列サイズ
     */
    vector(int n):num(n), x(new T[n]){
        for(int i = 0; i < num; i++){
            x[i] = T();
        }
    }
    /**
     * @brief コピーコンストラクタ
     * @param v コピー元
     */
    vector(const vector<T> &v):num(v.num), x(new T[num]){
        for(int i = 0; i < num; i++){
            x[i] = v[i];
        }
    }
    /**
     * @brief デストラクタ
     */
    ~vector( ){
        delete[] this->x;
    }
    
    /**
     * @brief 添字演算子
     * @param index インデックス
     * @return 要素への参照
     */
    T& operator [](int index){
        if(index < 0 || num <= index){
            throw "out of range";
        }
        return x[index];
    }
    /**
     * @brief 添字演算子（const版）
     * @param index インデックス
     * @return 要素へのconst参照
     */
    const T& operator [](int index) const{
        if(index < 0 || num <= index){
            throw "out of range";
        }
        return x[index];
    }
    /**
     * @brief ベクター同士の加算
     * @param v 加算するベクター
     * @return 加算結果
     */
    const vector<T> operator +(const vector<T> &v) const{
        vector<T> result(num);
        for(int i = 0; i < num; i++){
            result[i] = x[i] + v[i];
        }
        return result;
    }
    /**
     * @brief ベクター同士の減算
     * @param v 減算するベクター
     * @return 減算結果
     */
    const vector<T> operator -(const vector<T> &v) const{
        vector<T> result(num);
        for(int i = 0; i < num; i++){
            result[i] = x[i] - v[i];
        }
        return result;
    }
    /**
     * @brief 代入演算子
     * @param v 代入元
     */
    void operator =(const vector<T> &v){
        if(this == &v){
            return;
        }
        delete[] x;
        x = new T[v.num];
        num = v.num;
        for(int i = 0; i < num; i++){
            x[i] = v[i];
        }
    }

    /**
     * @brief 配列サイズを返す
     * @return 配列サイズ
     */
    int size() const {
        return num;
    }
    /**
     * @brief 配列内容を標準出力に表示
     */
    virtual void print( ) const {
        if(this->x == NULL){
            std::cout << "Empty" << std::endl;
        }
        for(int i = 0; i < num; i++){
            std::cout << x[i] << std::endl;
        }
    }

protected: 
    int num; ///< 配列サイズ
    T* x;    ///< 配列本体
};

/**
 * @class matrix
 * @brief 2次元配列の自作マトリックスクラス
 * @tparam T 要素型
 */
template< class T >
class matrix : public vector<T>
{
public:
    /**
     * @brief 行・列指定コンストラクタ
     * @param r 行数
     * @param c 列数
     */
    matrix(int r, int c): vector<T>(r*c), num1(r), num2(c){}
    /**
     * @brief コピーコンストラクタ
     * @param m コピー元
     */
    matrix(const matrix<T> &m): vector<T>(m.size()), num1(m.rows()), num2(m.cols()){
        for(int r = 0; r < num1; r++){
            for(int c = 0; c < num2; c++){
                (*this)[c + r * num2] = m(r, c);
            }
        }
    }

    /**
     * @brief 2次元添字演算子
     * @param r 行
     * @param c 列
     * @return 要素への参照
     */
    T& operator ()( int r, int c ){
        if(r < 0 || c < 0 || num1 <= r || num2 <= c){
            throw "out of range";
        }
        return( (*this)[c + r * num2] );
    }
    /**
     * @brief 2次元添字演算子（const版）
     * @param r 行
     * @param c 列
     * @return 要素へのconst参照
     */
    const T& operator ()( int r, int c ) const{
        if(r < 0 || c < 0 || num1 <= r || num2 <= c){
            throw "out of range";
        }
        return( (*this)[c + r * num2] );
    }
    /**
     * @brief 代入演算子
     * @param m 代入元
     */
    void operator =(const matrix<T> &m){
        if(this == &m){
            return;
        }
        delete[] this->x;
        this->x = new T[m.size()];
        num1 = m.rows();
        num2 = m.cols();
        for(int r = 0; r < num1; r++){
            for(int c = 0; c < num2; c++){
                (*this)[c + r * num2] = m(r, c);
            }
        }
    }

    /**
     * @brief 行数を返す
     * @return 行数
     */
    int rows() const {
        return num1;
    }
    /**
     * @brief 列数を返す
     * @return 列数
     */
    int cols() const {
        return num2;
    }
    /**
     * @brief 配列内容を標準出力に表示
     */
    void print( ) const {
        if(this->x == NULL){
            std::cout << "Empty" << std::endl;
        }
        for(int r = 0; r < num1; r++){
            for(int c = 0; c < num2; c++){
                std::cout << (*this)[c + r * num2];
            }
            std::cout << std::endl;
        }
    }

protected:
    int num1; ///< 行数
    int num2; ///< 列数
};

/**
 * @brief vectorの内容を表示する関数
 * @tparam T 要素型
 * @param v 表示するvector
 */
template< class T >
void print(const vector<T> &v ){
    v.print( );
}

/**
 * @class PuyoArray
 * @brief ぷよぷよの盤面を管理するクラス
 * @tparam T 要素型
 */
template< class T >
class PuyoArray : public matrix<T> {
public:
    typedef matrix<T> base;

    /**
     * @brief デフォルトコンストラクタ
     */
    PuyoArray();
    /**
     * @brief サイズ指定コンストラクタ
     * @param rows 行数
     * @param cols 列数
     */
    PuyoArray(int rows, int cols) : base(rows, cols){}
    /**
     * @brief コピーコンストラクタ
     * @param puyo コピー元
     */
    PuyoArray(const PuyoArray &puyo) : base(puyo){}

public:
    /**
     * @brief 代入演算子
     * @param puyo 代入元
     */
    void operator =(const PuyoArray &puyo) {
        base::operator =(puyo);
    }

    /**
     * @brief 盤面をクリア
     */
    void clear(){
        for(int r = 0; r < (*this).rows(); r++){
            for(int c = 0; c < (*this).cols(); c++){
                (*this)(r,c) = NONE;
            }
        }
    }

    /**
     * @brief 2つのぷよ配列で同じ位置にぷよが存在する場合は true を返す
     * @param puyo 比較対象
     * @return 同じ位置にぷよがあればtrue、1つもなければfalseを返す
     */
    bool operator &&(const PuyoArray &puyo) const {
        if (puyo.size() != this->size()) return false;
        bool flag = false;
        for (int i = 0; i < this->size(); i++) {
            if (puyo[i] != NONE && this->operator[](i) != NONE) flag = true;
        }
        return flag;
    }

    /**
     * @brief 2つのぷよ配列をマージして新しいぷよ配列を生成
     * @param puyo マージ対象
     * @return マージ結果
     */
    const PuyoArray operator |(const PuyoArray &puyo) const {
        if (puyo.size() != this->size()) return *this;
        PuyoArray temp(*this);
        for (int i = 0; i < this->size(); i++) {
            if (puyo[i] != NONE) temp[i] = puyo[i];
        }
        return temp;
    }

    /**
     * @brief 盤面上のぷよの総数を返す
     * @return ぷよの数
     */
    int count() const {
        const base &mat = *this;
        int num = 0;
        for (int i = 0; i < this->size(); i++) {
            if (mat[i] > 0) num++;
        }
        return num;
    }

public:

    int rotate_count = 0; ///< 回転状態
    bool generate = false; ///< ぷよ生成可否
    int score_count = 0; ///< スコア
    int highscore = 0;
    int chain_count = 0;
    int next1 = 0;
    int next2 = 0;

     /**
     * @brief 盤面に新しいぷよ生成
     * @param stack 既存のぷよ配置
     */
    void GeneratePuyo(const PuyoArray &stack, int next1, int next2) {
        PuyoArray &active = *this;
        // 初期コードはRGのぷよを生成するコードとなっています
        // ランダムなぷよを生成するように修正してください
        generate = false;

        if(stack(0,2) == NONE && stack(0,3) == NONE){
            active(0, 2) = next1;
            active(0, 3) = next2;
            generate = true;
        }else{
            for(int c = 0; c < stack.cols() - 1; c++){
                if(stack(0,c) == NONE && stack(0,c + 1) == NONE){
                    active(0, c) = next1;
                    active(0, c + 1) = next2;
                    generate = true;
                    break;
                }
            }
        }

        rotate_count = 0;
    }

    /**
     * @brief ぷよの着地判定．着地したぷよの数を返す
     * @param stack 着地先
     * @return 着地したぷよの数
     */
    int MoveLandedPuyo(PuyoArray &stack) {
        PuyoArray &active = *this;  // アクティブ側のぷよ配列
        int num_landed = 0;         // 着地したぷよの数を数える変数

        // 1番下に到達したぷよを着地と判定する
        {
            int bottom = active.rows() - 1;

            for(int r = active.rows()-2; r >= 0; r--){
                for (int c = active.cols()-1; c >= 0; c--) {
                    if (active(bottom, c) != NONE) {
                         // 着地判定されたぷよをstackに移動する
                        stack(bottom, c) = active(bottom, c);
                        active(bottom, c) = NONE;
                        num_landed++;
                    }
                    if (stack(r+1,c) != NONE && active(r,c) != NONE) {
                        // 着地判定されたぷよをstackに移動する
                        stack(r, c) = active(r, c);
                        active(r, c) = NONE;
                        num_landed++;
                    }
                }
            }
        }

        return num_landed;
    }

    /**
     * @brief 四つ以上連結したぷよを消す
     * @param erased 消去したぷよの記録
     * @return 消去したぷよの数
     */
    int ErasePuyo(PuyoArray &erased){
        int blink = 4;
        for(int r = 0; r < (*this).rows(); r++){
            for(int c = 0; c < (*this).cols(); c++){
                if((*this)(r, c) == NONE){
                    continue;
                }
                PuyoArray tstack(*this), terased((*this).rows(), (*this).cols());
                terased(r, c) = tstack(r, c);
                tstack.ErasePuyo(terased, r, c, tstack(r, c));
                tstack(r, c) = NONE;
                if(terased.count() >= 4){
                    chain_count++;
                    erased = erased | terased;
                    *this = tstack;
                    for(int i = 0; i < blink; i++){
                    display(tstack | terased, (*this).score_count, (*this).highscore, (*this).chain_count, (*this).next1, (*this).next2);
                    usleep(GMCLOCK*2);
                    display(tstack, (*this).score_count, (*this).highscore, (*this).chain_count, (*this).next1, (*this).next2);
                    usleep(GMCLOCK*2);
                    }
                }
            }
        }
        return erased.count();
    }

    /**
     * @brief 消すぷよを調べる（再帰）
     * @param erased 消去記録
     * @param r 行
     * @param c 列
     * @param puyocolor 色
     */
    void ErasePuyo(PuyoArray &erased, int r, int c, int puyocolor){
        if((r - 1 >= 0) && (*this)(r-1, c) == puyocolor){
            erased(r-1, c) = (*this)(r-1, c);
            (*this)(r-1, c) = NONE;
            ErasePuyo(erased, r-1, c, puyocolor);
        }
        if((c - 1 >= 0) && (*this)(r, c-1) == puyocolor){
            erased(r, c-1) = (*this)(r, c-1);
            (*this)(r, c-1) = NONE;
            ErasePuyo(erased, r, c-1, puyocolor);
        }
        if((r + 1 < (*this).rows()) && (*this)(r+1, c) == puyocolor){
            erased(r+1, c) = (*this)(r+1, c);
            (*this)(r+1, c) = NONE;
            ErasePuyo(erased, r+1, c, puyocolor);
        }
        if((c + 1 < (*this).cols()) && (*this)(r, c+1) == puyocolor){
            erased(r, c+1) = (*this)(r, c+1);
            (*this)(r, c+1) = NONE;
            ErasePuyo(erased, r, c+1, puyocolor);
        }

    }

    /**
     * @brief 盤面のぷよを下に落とす
     * @param active アクティブぷよ
     * @return ぷよが動いたらtrue
     */
    bool MoveStack(const PuyoArray &active){
        bool loop = true;
        bool moved = false;
        while(loop){
            PuyoArray &stack = *this;  // アクティブ側のぷよ配列
            PuyoArray temp(stack);     // 一時的なぷよ格納配列
            loop = false;
            for(int r = temp.rows() - 1; r > 0; r--){
                for(int c = temp.cols() - 1; c >= 0; c--) {
                    if(temp(r, c) == NONE && temp(r-1, c) != NONE){
                        temp(r, c) = temp(r-1, c);
                        temp(r-1, c) = NONE;
                        loop = true;
                        moved = true;
                    }
                }                      
            }
            // アクティブ側にコピー
            stack = temp;
            display(active | stack, stack.score_count, stack.highscore, stack.chain_count, stack.next1, stack.next2);
            usleep(GMCLOCK);
        }
        return moved;
    }

    /**
     * @brief 左移動
     * @param stack 固定ぷよ
     */
    void MoveLeft(const PuyoArray &stack) {
        PuyoArray &active = *this;  // アクティブ側のぷよ配列
        PuyoArray temp(active);     // 一時的なぷよ格納配列

        // 1つ左の位置にぷよを移動させる
        for (int r = 0; r < temp.rows(); r++) {
            for (int c = 1; c < temp.cols(); c++) {
                if (temp(r, c) != NONE && temp(r, c - 1) == NONE && stack(r, c-1) == NONE) {
                    temp(r, c - 1) = temp(r, c);    // 一つ左にぷよをコピー
                    temp(r, c) = NONE;              // 元の位置のぷよは消す
                }
            }
        }

        for (int r = 0; r < temp.rows(); r++) {
            for (int c = 0; c < temp.cols(); c++) {
                if(temp(r,c) != NONE && stack(r,c) != NONE){
                    return;
                }
            }
        }

        // アクティブ側にコピー
        active = temp;
    }

    /**
     * @brief 右移動
     * @param stack 固定ぷよ
     */
    void MoveRight(const PuyoArray &stack) {
        PuyoArray &active = *this;  // アクティブ側のぷよ配列
        PuyoArray temp(active);     // 一時的なぷよ格納配列

        // 1つ右の位置にぷよを移動させる
        for (int r = 0; r < temp.rows(); r++) {
            for (int c = temp.cols() - 2; c >= 0; c--) {
                if (temp(r, c) != NONE && temp(r, c + 1) == NONE && stack(r, c+1) == NONE) {
                    temp(r, c + 1) = temp(r, c);    // 一つ右にぷよをコピー
                    temp(r, c) = NONE;              // 元の位置のぷよは消す
                }
            }
        }

        for (int r = 0; r < temp.rows(); r++) {
            for (int c = 0; c < temp.cols(); c++) {
                if(temp(r,c) != NONE && stack(r,c) != NONE){
                    return;
                }
            }
        }

        // アクティブ側にコピー
        active = temp;
    }

    /**
     * @brief 下移動
     * @param stack 固定ぷよ
     */
    void MoveDown(const PuyoArray &stack) {
        PuyoArray &active = *this;  // アクティブ側のぷよ配列
        PuyoArray temp(active);     // 一時的なぷよ格納配列

        // 1つ下の位置にぷよを移動させる
        for (int r = temp.rows() - 2; r >= 0; r--) {
            for (int c = 0; c < temp.cols(); c++) {
                if (temp(r, c) != NONE && temp(r + 1, c) == NONE) {
                    temp(r + 1, c) = temp(r, c);    // 一つ下にぷよをコピー
                    temp(r, c) = NONE;              // 元の位置のぷよは消す

                }
            }
        }

        for (int r = 0; r < temp.rows(); r++) {
            for (int c = 0; c < temp.cols(); c++) {
                if(temp(r,c) != NONE && stack(r,c) != NONE){
                    return;
                }
            }
        }
        // アクティブ側にコピー
        active = temp;
    }

    /**
     * @brief 回転
     * @param stack 固定ぷよ
     */
    void Rotate(const PuyoArray &stack) {
        PuyoArray &active = *this;  // アクティブ側のぷよ配列
        PuyoArray temp(active);     // 一時的なぷよ格納配列

        bool loop = false;
        // ぷよを回転させる
        for (int r = 0; r < temp.rows(); r++) {
            if(loop){
                break;
            }
            for (int c = 0; c < temp.cols(); c++) {
                if (temp(r, c) != NONE && rotate_count == 0 && (r + 1) != temp.rows()) {
                    temp(r + 1, c) = temp(r, c + 1);
                    temp(r, c + 1) = NONE;
                    rotate_count++;
                    loop = true;
                    break;
                } else if (temp(r, c) != NONE && rotate_count == 1 && (c - 1) != -1) {
                    temp(r, c - 1) = temp(r + 1, c);
                    temp(r + 1, c) = NONE;
                    rotate_count++;
                    loop = true;
                    break;
                } else if (temp(r, c) != NONE && rotate_count == 2 && (r - 1) != -1) {
                    temp(r - 1, c + 1) = temp(r, c);
                    temp(r, c) = NONE;
                    rotate_count++;
                    loop = true;
                    break;
                } else if (temp(r, c) != NONE && rotate_count == 3 && (c + 1) != temp.cols()) {
                    temp(r + 1, c + 1) = temp(r, c);
                    temp(r , c) = NONE;
                    rotate_count = 0;
                    loop = true;
                    break;
                }
            }
        }

        for (int r = 0; r < temp.rows(); r++) {
            for (int c = 0; c < temp.cols(); c++) {
                if(temp(r,c) != NONE && stack(r,c) != NONE){
                    rotate_count--;
                    return;
                }
            }
        }

        active = temp;
    }
};

/**
 * @brief 枠を描画
 * @param r1 左上行
 * @param c1 左上列
 * @param r2 右下行
 * @param c2 右下列
 * @param color 色
 */
void draw_rectangle(int r1, int c1, int r2, int c2, int color)
{
    attrset(color);
    mvhline(r1, c1, 0, c2 - c1);
    mvhline(r2, c1, 0, c2 - c1);
    mvvline(r1, c1, 0, r2 - r1);
    mvvline(r1, c2, 0, r2 - r1);
    mvaddch(r1, c1, ACS_ULCORNER);
    mvaddch(r2, c1, ACS_LLCORNER);
    mvaddch(r1, c2, ACS_URCORNER);
    mvaddch(r2, c2, ACS_LRCORNER);
}

/**
 * @brief 画面表示
 * @param puyo 表示する盤面
 * @param score_count スコア
 */
void display(const PuyoArray<int> &puyo, int score_count, int highscore, int chain_count, int next1, int next2) {
    // 枠を描画
    draw_rectangle(1, 0, puyo.rows() + 2, puyo.cols() * 2 + 2, COLOR_PAIR(0));

    // 落下中ぷよ表示
    for (int r = 0; r < puyo.rows(); r++) {
        for (int c = 0; c < puyo.cols(); c++) {
            switch (puyo(r, c)) {
            case NONE:
                attrset(COLOR_PAIR(0));
                mvaddch(r + 2, c * 2 + 2, '.');
                break;
            case RED:
                attrset(COLOR_PAIR(1));
                mvaddch(r + 2, c * 2 + 2, 'R');
                break;
            case BLUE:
                attrset(COLOR_PAIR(2));
                mvaddch(r + 2, c * 2 + 2, 'B');
                break;
            case GREEN:
                attrset(COLOR_PAIR(3));
                mvaddch(r + 2, c * 2 + 2, 'G');
                break;
            case YELLOW:
                attrset(COLOR_PAIR(4));
                mvaddch(r + 2, c * 2 + 2, 'Y');
                break;
            default:
                attrset(COLOR_PAIR(0));
                mvaddch(r + 2, c * 2 + 2, '?');
                break;
            }
        }
    }

    // 画面左上にスコアを表示
    char smsg[256];
    snprintf(smsg, 256, "Score: %d", score_count);
    attrset(COLOR_PAIR(0));
    mvaddstr(0, 1, smsg);
    char hsmsg[256];
    snprintf(hsmsg, 256, "HighScore: %d", highscore);
    mvaddstr(0, 15, hsmsg);
    char cmsg[64];
    snprintf(cmsg, 256, "Chain: %d", chain_count);
    mvaddstr(0, 33, cmsg);
    mvaddstr(2, 16, "Next:");
    char nmsg1[32];
    snprintf(nmsg1, 32, "%c", next1 == RED ? 'R' : next1 == BLUE ? 'B' : next1 == GREEN ? 'G' : 'Y');
    attrset(COLOR_PAIR(next1));
    mvaddstr(2, 21, nmsg1);
    char nmsg2[4];
    snprintf(nmsg2, 4, "%c", next2 == RED ? 'R' : next2 == BLUE ? 'B' : next2 == GREEN ? 'G' : 'Y');
    attrset(COLOR_PAIR(next2));
    mvaddstr(2, 22, nmsg2);
    char msg[64]; 
    snprintf(msg, 64, "<-/->:Move Down:Drop");
    attrset(COLOR_PAIR(0));
    mvaddstr(4, 17, msg);
    char msg2[64];
    snprintf(msg2, 64, "z:Rotate q:Quit");
    mvaddstr(5, 17, msg2);

    refresh();
}

/**
 * @brief メイン関数
 */
int main(int argc, char **argv) {
    try{

    // 画面の初期化
    initscr();

    // カラー属性を扱うための初期化
    start_color();

    // キーを押しても画面に表示しない
    noecho();

    // キー入力を即座に受け付ける
    cbreak();
    curs_set(0);

    // キー入力受付方法指定
    keypad(stdscr, TRUE);

    //色の作成
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    // キー入力非ブロッキングモード
    timeout(0);

    PuyoArray<int> active(12, 6);                          // 縦12マス✕横6マスで初期化
    PuyoArray<int> stack(active.rows(), active.cols());    // activeと同じサイズで初期化

    // 最初のぷよ生成
    std::random_device rd;
    std::mt19937 gen(rd());       
    std::uniform_int_distribution<> dist(1, 4);
    stack.next1 = dist(gen);
    stack.next2 = dist(gen);
    active.GeneratePuyo(stack, stack.next1, stack.next2);
    stack.next1 = dist(gen);
    stack.next2 = dist(gen);
    PuyoArray<int> erased(active.rows(), active.cols());

    int delay = 0;
    int waitCount = 6;

    const char* highscore_file = "highscore.txt";
    std::ifstream ifs(highscore_file);
    ifs >> stack.highscore;

    // メイン処理ループ
    bool loop = true;
    while (loop) {
        int act = active.count();
        // キー入力受付
        int ch = getch();

        // 入力キーごとの処理
        switch (ch) { 
        case KEY_LEFT:
            if(act == 2){
                active.MoveLeft(stack);
            }
            break;
        case KEY_RIGHT:
            if(act == 2){
                active.MoveRight(stack);
            }
            break;
        case KEY_DOWN:
            active.MoveDown(stack);
            break;
        case 'Z':
        case 'z':
            if(act == 2){
                active.Rotate(stack);
            }
            break;
        case 'q':  // q or Q の入力で終了する
        case 'Q':
            loop = false;
            continue;
        default:
            break;
        }

        // 処理速度調整のためのif文
        if (delay % waitCount == 0) {
            
            // ぷよ下に移動
            active.MoveDown(stack);

            // ぷよ着地判定
            if(active.MoveLandedPuyo(stack) > 0){
                act = active.count();

                // 着地していたら新しいぷよ生成
                if(act == 0){
                    bool loop2 = true;
                    while(loop2){
                        loop2 = false;
                        erased.clear();
                        if(stack.ErasePuyo(erased) > 0){
                            stack.score_count += erased.count();
                            if(stack.MoveStack(active)){
                                loop2 = true;
                            }   
                        }
                    }
                    stack.chain_count = 0;
                    usleep(GMCLOCK * 5);
                    active.GeneratePuyo(stack, stack.next1, stack.next2);
                    stack.next1 = dist(gen);
                    stack.next2 = dist(gen);                   
                }   
            }
        }
        delay++;

        // 表示
        display(active | stack, stack.score_count, stack.highscore, stack.chain_count, stack.next1, stack.next2);

        if(active.generate != true){
            if(stack.score_count > stack.highscore) {
                stack.highscore = stack.score_count;
                std::ofstream ofs(highscore_file);
                ofs << stack.highscore;
                if (!ofs) {
                    std::cerr << "Failed to open highscore file\n";
                }
            }
            std::cout << "\n" << "  game over. press q to end." << std::endl;
            bool loop3 = true;
            while(loop3){
                int ch = getch();
                switch(ch){
                    case 'q':  // q or Q の入力で終了する
                    case 'Q':
                        loop = false;
                        loop3 = false;
                        break;
                }
            }
        }

        // ゲームの進行速度を調整するためのスリープ
        usleep(GMCLOCK);
    }

    // 画面をリセット
    endwin();

    return 0;
    }catch (const char* msg) {
        endwin();
        std::cerr << "例外発生: " << msg << std::endl;
        return 1;
    }
}
