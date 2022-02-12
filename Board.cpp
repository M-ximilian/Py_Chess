//
// Created by maxim on 12.02.2022.
//

#include "Board.h"

tuple<int *, bool, bool *, int, int, int> convert_fen_to_position(string fen) {
    int board[64] = {0};
    bool first_moving_player = true;
    bool castling_rights[4] = {false};
    int en_passant_square = -1;
    int fifty_move_rule_counter;
    int move_count;

    int index = 0;
    // get board
    int board_index = 0;
    while (fen[index] != ' ') {
        if (fen[index] == '/') { index++; }
        int piece = convert_piece(fen[index]);
        if (piece != 0) {
            board[8 * (7 - board_index / 8) + board_index % 8] = piece;
            board_index++;
        } else {
            board_index += (int) fen[index] - 48;
        }
        index++;
    }
    index += 2;
    //get first player to move
    if (fen[index] == 'b') { first_moving_player = false; }
    index++;
    //get castling rights
    if (fen[index] != '-') {
        while (fen[index] != ' ') {
            switch (fen[index]) {
                case 'k':
                    castling_rights[0] = true;
                    break;
                case 'q':
                    castling_rights[1] = true;
                    break;
                case 'K':
                    castling_rights[2] = true;
                    break;
                case 'Q':
                    castling_rights[3] = true;
                    break;
            }
            index++;
        }
    } else {
        index++;
    }
    index++;
    //  get en passant square
    if (fen[index] != '-') {
        en_passant_square = (int) fen[index] - 97 + 8 * (fen[index + 1] - 49);
        index += 3;
    } else {
        index += 2;
    }
    fifty_move_rule_counter = (int) fen[index] - 48;
    move_count = (int) fen[index + 2] - 48;


    return {board, first_moving_player, castling_rights, en_passant_square, fifty_move_rule_counter, move_count};
}

Board::Board(string fen_init) {

    // converts starting position
    if (fen_init == "start" || fen_init == "startpos") {
        fen_init = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    tuple<int *, bool, bool *, int, int, int> converted_fen = convert_fen_to_position(std::move(fen_init));

    // set board values + positions- and existence maps from gathered information
    for (int piece = 0; piece < 64; piece++) {
        board[piece] = Piece(get<0>(converted_fen)[piece] / 7, get<0>(converted_fen)[piece] % 7);
        existence_map[piece] = board[piece].get_type() != none;
        if (existence_map[piece]) {
            color_map[piece] = board[piece].get_color();
        }
        if (get<0>(converted_fen)[piece] != 0) { amount_of_pieces++; }
    }
    current_player = get<1>(converted_fen);
    for (int castling_direction = 0; castling_direction < 4; castling_direction++) {
        castling_rights[castling_direction] = get<2>(converted_fen)[castling_direction];
    }
    en_passant_square = get<3>(converted_fen);
    fifty_moves_rule_count = get<4>(converted_fen);
    move_count = get<5>(converted_fen);
}

void Board::generate_piece_moves() {}