//
// Created by maxim on 12.02.2022.
//

#ifndef PY_CHESS_BOARD_H
#define PY_CHESS_BOARD_H

#include "Piece.h"


class Board {
public:
    Board(string);

    void generate_piece_moves();

private:
    Piece board[64];

    vector<int> white_positions;
    vector<int> black_positions;

    int king_positions[2]{};
    bool current_player;
    bool castling_rights[4]{false}; //kqKQ
    int castling_end_squares[4] = {62, 58, 61, 59}; // first king then rook, first short then long for black
    int en_passant_square = -1;
    bool en_passant_updated_this_move = false;
    int fifty_moves_rule_count;
    int move_count;
    bool en_passant_from_fen = true;
    int undo_count = 0;
    int amount_of_pieces = 0;
    vector<stored_move> move_history{};
    stored_move no_move = stored_move{-1, -1,none, none, -1, -1, {false, false, false, false}};

    // first location of piece, then viewing direction in which piece is pinned/ checked, needed for blocking
    vector<tuple<int,int>> checking{};
    vector<tuple<int,int, int>> pinning{}; // marker for en passant pins
    vector<tuple<int, int, int>> en_passant_pinning{};
    unordered_set<int> defended_squares;

    const short int knight_moving_directions[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
    const short int sliding_move_directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

    stored_move * last_move(){if (move_history.size()-undo_count == 0){return &no_move;} else {return &move_history.at(move_history.size()-undo_count-1);}};

};


#endif //PY_CHESS_BOARD_H
