//
// Created by maxim on 12.02.2022.
//

#ifndef PY_CHESS_MAIN_H
#define PY_CHESS_MAIN_H

#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <ctime>
#include <chrono>

using namespace std;
enum piece_types {
    none, pawn, knight, bishop, rook, queen, king
};
struct move {
    int starting_square, destination_square, promotion_type;
};
struct stored_move {
    int previews_square, new_square;
    piece_types piece, taken_piece;
    int piece_taken_at;
    int en_passant_square;
    bool castling_rights[4];
};
#endif //PY_CHESS_MAIN_H
