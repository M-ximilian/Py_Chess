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
    bool castling_rights[4]{false};
    int castling_end_squares[4] = {6, 2, 5, 3}; // first king then rook, first short then long
    int en_passant_square = -1;
    int fifty_moves_rule_count;
    int move_count;
    int amount_of_pieces = 0;

    // first location of piece, then viewing direction in which piece is pinned/ checked, needed for blocking
    vector<tuple<int,int>> checking[2]{};
    vector<tuple<int,int, bool>> pinning[2]{}; // marker for en passant pins
};


#endif //PY_CHESS_BOARD_H
