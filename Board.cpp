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
    index++;
    //get first player to move
    if (fen[index] == 'b') { first_moving_player = false; }
    index += 2;
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

    int current_move_position; // counts the index in move-array
    bool castling_rights_this_move[2]{true};

    vector<int> &opponent_pieces = (current_player ? black_positions : white_positions);
    vector<int> &own_pieces = (current_player ? white_positions : black_positions);
    // get only collision detection and pinning/ checking
    bool only_searching_pins; // active if collision detected but pin still possible
    bool en_passant_pin_exception = false; // if true, a pair of pawns has already been found
    for (auto opponent_piece_position: opponent_pieces) {
        current_move_position = 0;
        Piece &opponent_piece = board[opponent_piece_position];
        if (opponent_piece.get_type() == pawn) {
            for (auto destination_square: {-9 + 16 * current_player, -7 + 16 * current_player}) {
                if (abs(opponent_piece_position & 8 - destination_square % 8) != 1) { continue; }
                // disable castling if opponent sees part of king castling route
                // short
                if (castling_rights[2 * current_player] && castling_rights_this_move[0]) {
                    if (destination_square <= castling_end_squares[0] - 56 * current_player &&
                        destination_square > king_positions[current_player]) {
                        castling_rights_this_move[0] = false;
                    }
                    //long
                } else if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1] &&
                           opponent_piece.get_color() != current_player) {
                    if (destination_square >= castling_end_squares[1] - 56 * current_player &&
                        destination_square < king_positions[current_player]) {
                        castling_rights_this_move[1] = false;
                    }
                } else if (destination_square == king_positions[current_player]) {
                    checking.emplace_back(opponent_piece_position, destination_square);
                    break;
                }
            }
        } else if (opponent_piece.get_type() == knight) {
            int moving_directions[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
            for (auto direction: moving_directions) {
                int destination_square = opponent_piece_position + direction;
                Piece &destination_piece = board[destination_square];

                // skip moves that would leave board
                if (!(abs(destination_square / 8 - opponent_piece_position / 8) == 2 &&
                      abs(destination_square % 8 - opponent_piece_position % 8) == 1 ||
                      !(abs(destination_square / 8 - opponent_piece_position / 8) == 1 &&
                        abs(destination_square % 8 - opponent_piece_position % 8) == 2))) {
                    continue;
                }

                // disable castling if opponent sees part of king castling route
                //short
                if (castling_rights[2 * current_player] && castling_rights_this_move[0]) {
                    if (destination_square <= castling_end_squares[0] - 56 * current_player &&
                        destination_square > king_positions[current_player]) {
                        castling_rights_this_move[0] = false;
                    }
                    //long
                } else if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1]) {
                    if (destination_square >= castling_end_squares[1] - 56 * current_player &&
                        destination_square < king_positions[current_player]) {
                        castling_rights_this_move[1] = false;
                    }
                } else if (destination_square == king_positions[current_player]) {
                    checking.emplace_back(opponent_piece_position, destination_square);
                    castling_rights_this_move[0] = false;
                    castling_rights_this_move[1] = false;
                }
            }

        } else if (opponent_piece.get_type() == bishop || opponent_piece.get_type() == rook ||
                   opponent_piece.get_type() == queen) {

            // see if castling lanes are empty in case of rook, later for own pieces
            //king side
            /*
            if (castling_rights[2 * current_player] && castling_rights_this_move[0] &&
                opponent_piece.get_type() == rook && opponent_piece.get_color() == current_player &&
                opponent_piece_position > king_positions[current_player]) {
                bool king_present = false, rook_present = false;
                for (int tile = min(castling_end_squares[2] - 56 * current_player, king_positions[current_player]);
                     tile < max(castling_end_squares[0] - 56 * current_player, opponent_piece_position); tile++) {
                    if (board[tile].get_type() == none) {
                        continue;
                    } else if (!king_present && board[tile].get_color() == current_player &&
                               board[tile].get_type() == king) {
                        king_present = true;
                        continue;
                    } else if (!rook_present && board[tile].get_color() == current_player &&
                               board[tile].get_type() == rook) {
                        rook_present = true;
                        continue;
                    } else {
                        castling_rights_this_move[0] = false;
                        break;
                    }
                }
            }

            //queen side
            if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1] &&
                opponent_piece.get_type() == rook && opponent_piece.get_color() == current_player &&
                opponent_piece_position < king_positions[current_player]) {
                bool king_present = false, rook_present = false;
                for (int tile = min(castling_end_squares[1] - 56 * current_player, opponent_piece_position); tile <
                                                                                                 max(castling_end_squares[3] -
                                                                                                     56 *
                                                                                                     current_player,
                                                                                                     king_positions[current_player]); tile++) {
                    if (board[tile].get_type() == none) {
                        continue;
                    } else if (!king_present && board[tile].get_color() == current_player &&
                               board[tile].get_type() == king) {
                        king_present = true;
                        continue;
                    } else if (!rook_present && board[tile].get_color() == current_player &&
                               board[tile].get_type() == rook) {
                        rook_present = true;
                        continue;
                    } else {
                        castling_rights_this_move[1] = false;
                        break;
                    }
                }
            }
             */

            const short int move_directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};
            short int *sliding_lengths = sliding_piece_distances[opponent_piece.get_int_type() -
                                                                 3][opponent_piece_position];

            // go through every viewing direction
            for (int direction = 0; direction < 8; direction++) {
                const int &move_length = sliding_lengths[direction];
                const int &current_move_direction = move_directions[direction];

                only_searching_pins = false;
                en_passant_pin_exception = false;

                // go through the maximum amount of moves in said direction
                for (int opponent_move = 1; opponent_move <= move_length; opponent_move++) {
                    int destination_square = opponent_piece_position + opponent_move * current_move_direction;
                    Piece &destination_piece = board[destination_square];
                    if (!only_searching_pins) {
                        // disable castling if opponent sees part of king castling route
                        // short
                        if (castling_rights[2 * current_player] && castling_rights_this_move[0]) {
                            if (destination_square <= castling_end_squares[0] - 56 * current_player &&
                                destination_square > king_positions[current_player]) {
                                castling_rights_this_move[0] = false;
                            }
                            //long
                        } else if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1] &&
                                   opponent_piece.get_color() != current_player) {
                            if (destination_square >= castling_end_squares[1] - 56 * current_player &&
                                destination_square < king_positions[current_player]) {
                                castling_rights_this_move[1] = false;
                            }
                        } else if (destination_square == king_positions[current_player]) {
                            checking.emplace_back(opponent_piece_position, current_move_direction);
                            break;
                        }
                        // ---------------------------------------------------------------------

                        if (destination_piece.get_type() == none) {
                            continue;
                        } else if (destination_piece.get_color() == current_player) {
                            only_searching_pins = true;
                        } else if (destination_piece.get_color() != current_player) {
                            // checks if: 1) direction is sideways, 2) destination piece is an own pawn,
                            // 3) next square is still part of moving direction, 4) next square has opponent pawn
                            if (abs(current_move_direction) == 1 && destination_piece.get_type() == pawn &&
                                opponent_move != move_length &&
                                board[opponent_piece_position +
                                      (opponent_move + 1) * current_move_direction].get_type() ==
                                pawn &&
                                board[opponent_piece_position +
                                      (opponent_move + 1) * current_move_direction].get_color() ==
                                current_player) {
                                only_searching_pins = true;
                                en_passant_pin_exception = true;
                            } else {
                                break;
                            }
                        }
                    } else {
                        // only searching pins
                        if (destination_piece.get_type() == none) {
                            continue;
                        } else if (destination_piece.get_color() == current_player &&
                                   destination_piece.get_type() == king) {
                            pinning.emplace_back(opponent_piece_position, current_move_direction,
                                                 en_passant_pin_exception);
                        } else if (destination_piece.get_color() != current_player && !en_passant_pin_exception &&
                                   abs(current_move_direction) == 1 && destination_piece.get_type() == pawn &&
                                   board[opponent_piece_position +
                                         (opponent_move - 1) * current_move_direction].get_type() == pawn &&
                                   board[opponent_piece_position +
                                         (opponent_move - 1) * current_move_direction].get_color() ==
                                   current_player) {
                            // en passant case with first own piece then opponent piece
                            en_passant_pin_exception = true;
                            continue;
                        } else { break; }
                    }
                }
            }
        } else if (opponent_piece.get_type() == king) {
            int opponent_moves[8] = {-9, -8, -7, -1, 1, 7, 8, 9};
            int actual_opponent_moves[8];
            int actual_position_counter = 0;
            for (int &opponent_move: opponent_moves) {
                if (abs((opponent_move + opponent_piece_position) / 8 - opponent_piece_position / 8) <= 1 &&
                    abs((opponent_move + opponent_piece_position) % 8 - opponent_piece_position % 8) <= 1 &&
                    opponent_move + opponent_piece_position > -1 && opponent_move + opponent_piece_position < 64) {
                    actual_opponent_moves[actual_position_counter] = opponent_piece_position + opponent_move;
                    actual_position_counter++;
                }
            }
            opponent_piece.set_moves(actual_opponent_moves, actual_position_counter);
        } else { throw invalid_argument("received invalid opponent piece type"); }
    }
    // king collisions


}