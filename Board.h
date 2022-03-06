//
// Created by maxim on 12.02.2022.
//

#ifndef PY_CHESS_BOARD_H
#define PY_CHESS_BOARD_H

#include "Piece.h"


class Board {
public:
    Board(string);

    game_ends generate_piece_moves(bool perft = false);

    void undo();

    void run();

    void random_games(int count);

    int perft(int depth, map<string, int> * stored, const tuple<int, int, int>& last_moved = {});

    bool make_move(int starting_square, int destination_square, int promotion_type);

    void draw();

private:
    Piece board[64];

    unordered_set<int> white_positions;
    unordered_set<int> black_positions;

    int king_positions[2]{};
    bool current_player;
    bool castling_rights[4]{false}; //kqKQ
    int castling_end_squares[2][4] = {{62, 58, 61, 59},
                                      {6,  2,  5,  3}}; // ks, kl, rs, rl (k-king, r-rook, s-short, l-long)
    int original_castling_rook_positions[2][2] = {{64, -1},{64, -1}}; // KQkq
    int en_passant_square = -1;
    bool en_passant_updated_this_move = true;
    int fifty_moves_rule_count;
    int move_count;
    int undo_count = 0;
    vector<stored_move> move_history{};
    vector<stored_position> positions{};
    stored_move no_move = stored_move{-1, -1, none, none, -1, -1, {false, false, false, false}};

    // first location of piece, then viewing direction in which piece is pinned/ checked, needed for blocking
    vector<tuple<int, int>> checking{};
    vector<tuple<int, int, int>> pinning{}; // marker for en passant pins
    vector<tuple<int, int, int>> en_passant_pinning{};
    unordered_set<int> defended_squares;

    const short int knight_moving_directions[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
    const short int sliding_move_directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

    stored_move *last_move() {
        if (move_history.size() - undo_count == 0) { return &no_move; }
        else { return &move_history.at(move_history.size() - undo_count - 1); }
    };


    bool make_move(tuple<int, int, int> squares) {return make_move(get<0>(squares), get<1>(squares), get<2>(squares));}

    void store_current_position();

    void get_pgn();

    void update_en_passant();


};


#endif //PY_CHESS_BOARD_H
