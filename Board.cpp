//
// Created by maxim on 12.02.2022.
//

#include "Board.h"

tuple<bool, int, int, int> convert_fen_to_position(string fen, Piece *board, bool *castling_rights) {
    bool first_moving_player = true;
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
            board[8 * (7 - board_index / 8) + board_index % 8] = Piece(piece / 7, piece % 7);
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


    return {first_moving_player, en_passant_square, fifty_move_rule_counter, move_count};
}

Board::Board(string fen_init) {

    // converts starting position
    if (fen_init == "start" || fen_init == "startpos") {
        fen_init = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    tuple<bool, int, int, int> converted_fen = convert_fen_to_position(fen_init, board, castling_rights);

    // set board values + positions- and existence maps from gathered information
    int i = 0;
    for (auto piece: board) {
        if (piece.get_type() != none) {
            amount_of_pieces++;
            if (piece.get_type() == king) { king_positions[piece.get_color()] = i; }
            if (piece.get_color()) { white_positions.push_back(i); }
            else { black_positions.push_back(i); }
        }
        i++;
    }
    current_player = get<0>(converted_fen);
    en_passant_square = get<1>(converted_fen);
    fifty_moves_rule_count = get<2>(converted_fen);
    move_count = get<3>(converted_fen);
}

void Board::generate_piece_moves() {
    bool castling_rights_this_move{true};
    // get only collision detection and pinning/ checking
    bool only_searching_pins; // active if collision detected but pin still possible
    bool en_passant_pin_exception = false; // if true, pair of pawns already found
    int current_move_position; // counts the index in move-array

    for (auto white_piece_index: white_positions) {
        current_move_position = 0;
        Piece &white_piece = board[white_piece_index];
        if (white_piece.get_type() == pawn) {

        } else if (white_piece.get_type() == knight) {

        } else if (white_piece.get_type() == bishop || white_piece.get_type() == rook ||
                   white_piece.get_type() == queen) {
            const short int move_directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};
            short int *sliding_lengths = sliding_piece_distances[white_piece.get_int_type() - 3][white_piece_index];

            // go through every viewing direction
            for (int direction = 0; direction < 8; direction++) {
                const int &move_length = sliding_lengths[direction];
                const int &current_move_direction = move_directions[direction];

                only_searching_pins = false;

                // go through the maximum amount of moves in said direction
                for (int white_move = 1; white_move <= move_length; white_move++) {
                    int destination_square = white_piece_index + white_move * current_move_direction;
                    Piece &destination_piece = board[destination_square];
                    if (!only_searching_pins) {


                        // no piece at square -> add move
                        if (destination_piece.get_type() == none) {
                            white_piece.set_moves(current_move_position, destination_square);
                            current_move_position++;
                        } else if (!destination_piece.get_color()) {
                            // opponent piece at square -> add move and initiate "only pins"
                            white_piece.set_moves(current_move_position, destination_square);
                            current_move_position++;
                            if (destination_square == king_positions[0]) {
                                checking[1].emplace_back(white_piece_index, current_move_direction);
                                break;
                            } else {
                                only_searching_pins = true;
                            }
                        } else {
                            // checks if: 1) direction is sideways, 2) destination piece is an own pawn,
                            // 3) next square is still part of moving direction, 4) next square has opponent pawn
                            if (abs(current_move_direction) == 8 && destination_piece.get_type() == pawn &&
                                white_move != move_length &&
                                board[white_piece_index + (white_move + 1) * current_move_direction].get_type() ==
                                pawn &&
                                !board[white_piece_index + (white_move + 1) * current_move_direction].get_color()) {
                                only_searching_pins = true;
                                en_passant_pin_exception = true;
                            } else {
                                break;
                            }
                        }
                    } else {
                        // only searching pins
                        if (destination_piece.get_type() == none) { continue; }
                        else if (!destination_piece.get_color()) {
                            // black piece found
                            if (destination_piece.get_type() == king) {
                                pinning[1].emplace_back(white_piece_index, current_move_direction,
                                                        en_passant_pin_exception);
                            } else {
                                break;
                            }
                        } else {
                            // white piece found, break or go into enpassant case

                            // not already had exception, direction is horizontal, destination is white pawn,
                            // previews is pawn, previews is black
                            if (!en_passant_pin_exception && abs(current_move_direction) == 8 &&
                                destination_piece.get_type() == pawn &&
                                board[white_piece_index + (white_move - 1) * current_move_direction].get_type() ==
                                pawn &&
                                !board[white_piece_index + (white_move - 1) * current_move_direction].get_color()) {
                                en_passant_pin_exception = true;
                                continue;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        } else if (white_piece.get_type() == king) {

        } else { throw invalid_argument("received invalid white piece type"); }
    }
    // king collisions


}