//
// Created by maxim on 12.02.2022.
//

#include "Piece.h"

Piece::Piece(bool piece_color, piece_types piece_type) {
    color = piece_color;
    type = piece_type;
    amount_moves = 0;
}

Piece::Piece(bool piece_color, int piece_type) {
    color = piece_color;
    type = static_cast<piece_types>(piece_type);
    amount_moves = 0;
}

void Piece::set_moves(const int *new_moves, int amount) {
    for (int move = 0; move < amount; move++) {
        moves[move] = *(new_moves+move);
    }
    amount_moves = amount;
}

int convert_piece(char piece) {
    switch (piece) {
        case 'p':
            return 1;
        case 'n':
            return 2;
        case 'b':
            return 3;
        case 'r':
            return 4;
        case 'q':
            return 5;
        case 'k':
            return 6;
        case 'P':
            return 8;
        case 'N':
            return 9;
        case 'B':
            return 10;
        case 'R':
            return 11;
        case 'Q':
            return 12;
        case 'K':
            return 13;
        default:
            return 0;
    }
}
char convert_piece(Piece piece) {
    switch (piece.get_type()) {
        case pawn:
            return piece.get_color()?'P':'p';
        case knight:
            return piece.get_color()?'N':'n';
        case bishop:
            return piece.get_color()?'B':'b';
        case rook:
            return piece.get_color()?'R':'r';
        case queen:
            return piece.get_color()?'Q':'q';
        case king:
            return piece.get_color()?'K':'k';
        default:
            return '#';
    }
}

char convert_piece(piece_types piece) {
    switch (piece) {
        case pawn:
            return 'P';
        case knight:
            return 'N';
        case bishop:
            return 'B';
        case rook:
            return 'R';
        case queen:
            return 'Q';
        case king:
            return 'K';
        default:
            return ' ';
    }
}
