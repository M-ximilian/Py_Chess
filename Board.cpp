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
                case 'K':
                    castling_rights[0] = true;
                    break;
                case 'Q':
                    castling_rights[1] = true;
                    break;
                case 'k':
                    castling_rights[2] = true;
                    break;
                case 'q':
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
            if (piece.get_type() == king) { king_positions[piece.get_color()] = i; }
            if (piece.get_color()) { white_positions.insert(i); }
            else { black_positions.insert(i); }
        }
        i++;
    }
    // save rooks that could castle
    for (int r = 0; r < 64; r++) {
        if (board[r].get_type() == rook) {
            if (r < king_positions[board[r].get_color()]) {
                if (king_positions[board[r].get_color()] - r < king_positions[board[r].get_color()] -
                                                               original_castling_rook_positions[1 -
                                                                                                board[r].get_color()][1] &&
                    r / 8 == 7 * (1 - board[r].get_color())) {
                    original_castling_rook_positions[1 - board[r].get_color()][1] = r;
                }
            } else {
                if (r - king_positions[board[r].get_color()] <
                    original_castling_rook_positions[1 - board[r].get_color()][0] -
                    king_positions[board[r].get_color()]) {
                    original_castling_rook_positions[1 - board[r].get_color()][0] = r;
                }
            }
        }
    }
    // disable castling for every square where no rook is found
    if (original_castling_rook_positions[0][0] == 64) { castling_rights[0] = false; }
    if (original_castling_rook_positions[0][1] == -1) { castling_rights[1] = false; }
    if (original_castling_rook_positions[1][0] == 64) { castling_rights[2] = false; }
    if (original_castling_rook_positions[1][1] == -1) { castling_rights[3] = false; }

    // update all other variables
    current_player = get<0>(converted_fen);
    en_passant_square = get<1>(converted_fen);
    if (en_passant_square != -1) { en_passant_updated_this_move = true; }
    fifty_moves_rule_count = get<2>(converted_fen);
    move_count = get<3>(converted_fen);
    store_current_position();
}

game_ends Board::generate_piece_moves(bool perft) {
    // compute available pre update game end szenarios
    if (!perft) {
        if (fifty_moves_rule_count > 99) { return fifty_move_rule; }
        int count_pieces = 0;
        bool enough_material = false;
        int bishops[2] = {0, 0};
        bool knights = false;
        for (auto piece_index: white_positions) {
            Piece &piece = board[piece_index];
            if (piece.get_type() == pawn || piece.get_type() == rook || piece.get_type() == queen) {
                enough_material = true;
                break;
            }
            if (piece.get_type() == bishop) {
                count_pieces++;
                bishops[(piece_index / 8 % 2 + piece_index % 2) % 2]++;
                if (knights) {
                    enough_material = true;
                    break;
                }
            } else if (piece.get_type() == knight) {
                count_pieces++;
                knights = true;
                if (bishops[0] > 0 || bishops[1] > 0) {
                    enough_material = true;
                    break;
                }
            } else {
                count_pieces++;
            }
        }
        if (!enough_material) {
            for (auto piece_index: black_positions) {
                Piece &piece = board[piece_index];
                if (piece.get_type() == pawn || piece.get_type() == rook || piece.get_type() == queen) {
                    enough_material = true;
                    break;
                }
                if (piece.get_type() == bishop) {
                    count_pieces++;
                    bishops[(piece_index / 8 % 2 + piece_index % 2) % 2]++;
                    if (knights) {
                        enough_material = true;
                        break;
                    }
                } else if (piece.get_type() == knight) {
                    count_pieces++;
                    knights = true;
                    if (bishops[0] > 0 || bishops[1] > 0) {
                        enough_material = true;
                        break;
                    }
                } else {
                    count_pieces++;
                }

            }
        }
        if (!enough_material && (count_pieces < 4 || !knights && (bishops[0] == 0 || bishops[1] == 0))) {
            return insufficient_material;
        }
    }


    checking.clear();
    pinning.clear();
    en_passant_pinning.clear(), defended_squares.clear();
    int pinned_piece;

    bool castling_rights_this_move[2]{true, true};

    unordered_set<int> &opponent_pieces = (current_player ? black_positions : white_positions);
    unordered_set<int> &own_pieces = (current_player ? white_positions : black_positions);




    // get only collision detection and pinning/ checking
    bool first_piece_found; // active if collision detected but pin still possible
    bool en_passant_pin_exception = false; // if true, a pair of pawns has already been found
    for (auto opponent_piece_position: opponent_pieces) {
        Piece &opponent_piece = board[opponent_piece_position];
        if (opponent_piece.get_type() == pawn) {
            for (auto destination_square: {7 - 16 * current_player + opponent_piece_position,
                                           9 - 16 * current_player + opponent_piece_position}) {
                if (abs(opponent_piece_position % 8 - destination_square % 8) != 1) { continue; }
                // disable castling if opponent sees part of king castling route
                if (destination_square == king_positions[current_player]) {
                    checking.emplace_back(opponent_piece_position, destination_square - opponent_piece_position);
                    castling_rights_this_move[0] = false;
                    castling_rights_this_move[1] = false;
                } else {
                    // short
                    if (castling_rights[2 - 2 * current_player] && castling_rights_this_move[0]) {
                        if (destination_square <= castling_end_squares[current_player][0] &&
                            destination_square > king_positions[current_player]) {
                            castling_rights_this_move[0] = false;
                        }
                        //long
                    }
                    if (castling_rights[3 - 2 * current_player] && castling_rights_this_move[1]) {
                        if (destination_square >= castling_end_squares[current_player][1] &&
                            destination_square < king_positions[current_player]) {
                            castling_rights_this_move[1] = false;
                        }
                    }
                }
                if (abs(king_positions[current_player] - destination_square) == 1 ||
                    abs(king_positions[current_player] - destination_square) == 7 ||
                    abs(king_positions[current_player] - destination_square) == 8 ||
                    abs(king_positions[current_player] - destination_square) == 9) {
                    defended_squares.insert(destination_square);
                }
            }
        } else if (opponent_piece.get_type() == knight) {

            for (auto direction: knight_moving_directions) {
                int destination_square = opponent_piece_position + direction;
                Piece &destination_piece = board[destination_square];

                // skip moves that would leave board
                if (!(destination_square / 8 - opponent_piece_position / 8 + destination_square % 8 -
                      opponent_piece_position % 8 > -4 &&
                      destination_square / 8 - opponent_piece_position / 8 + destination_square % 8 -
                      opponent_piece_position % 8 < 4) || destination_square < 0 || destination_square > 63) {
                    continue;
                }

                // disable castling if opponent sees part of king castling route
                if (destination_square == king_positions[current_player]) {
                    checking.emplace_back(opponent_piece_position, direction);
                    castling_rights_this_move[0] = false;
                    castling_rights_this_move[1] = false;
                } //short
                else {
                    if (castling_rights[2 - 2 * current_player] && castling_rights_this_move[0]) {
                        if (destination_square <= castling_end_squares[current_player][0] &&
                            destination_square > king_positions[current_player]) {
                            castling_rights_this_move[0] = false;
                        }
                        //long
                    }
                    if (castling_rights[3 - 2 * current_player] && castling_rights_this_move[1]) {
                        if (destination_square >= castling_end_squares[current_player][1] &&
                            destination_square < king_positions[current_player]) {
                            castling_rights_this_move[1] = false;
                        }
                    }
                }
                if (abs(king_positions[current_player] - destination_square) == 1 ||
                    abs(king_positions[current_player] - destination_square) == 7 ||
                    abs(king_positions[current_player] - destination_square) == 8 ||
                    abs(king_positions[current_player] - destination_square) == 9) {
                    defended_squares.insert(destination_square);
                }
            }

        } else if (opponent_piece.get_type() == bishop || opponent_piece.get_type() == rook ||
                   opponent_piece.get_type() == queen) {
            short int *sliding_lengths = sliding_piece_distances[opponent_piece.get_type() -
                                                                 3][opponent_piece_position];

            // go through every viewing direction
            for (int direction = 0; direction < 8; direction++) {
                const int &move_length = sliding_lengths[direction];
                const int &current_move_direction = sliding_move_directions[direction];

                first_piece_found = false;
                en_passant_pin_exception = false;

                // go through the maximum amount of moves in said direction
                for (int opponent_move = 1; opponent_move <= move_length; opponent_move++) {
                    int destination_square = opponent_piece_position + opponent_move * current_move_direction;
                    Piece &destination_piece = board[destination_square];
                    if (!first_piece_found) {
                        // disable castling if opponent sees part of king castling route
                        if (destination_square == king_positions[current_player]) {
                            checking.emplace_back(opponent_piece_position, current_move_direction);
                            defended_squares.insert(king_positions[current_player] + current_move_direction);
                            castling_rights_this_move[0] = false;
                            castling_rights_this_move[1] = false;
                            break;
                        } else {
                            // short
                            if (castling_rights[2 - 2 * current_player] && castling_rights_this_move[0]) {
                                if (destination_square <= castling_end_squares[current_player][0] &&
                                    destination_square > king_positions[current_player]) {
                                    castling_rights_this_move[0] = false;
                                }
                                //long
                            }
                            if (castling_rights[3 - 2 * current_player] && castling_rights_this_move[1] &&
                                opponent_piece.get_color() != current_player) {
                                if (destination_square >= castling_end_squares[current_player][1] &&
                                    destination_square < king_positions[current_player]) {
                                    castling_rights_this_move[1] = false;
                                }
                            }
                        }

                        if (abs(king_positions[current_player] - destination_square) == 1 ||
                            abs(king_positions[current_player] - destination_square) == 7 ||
                            abs(king_positions[current_player] - destination_square) == 8 ||
                            abs(king_positions[current_player] - destination_square) == 9) {
                            defended_squares.insert(destination_square);
                        }
                        // ---------------------------------------------------------------------

                        if (destination_piece.get_type() == none) {
                            continue;
                        } else if (destination_piece.get_color() == current_player) {
                            pinned_piece = destination_square;
                            first_piece_found = true;
                        } else if (destination_piece.get_color() != current_player) {

                            // checks if: 1) direction is sideways, 2) destination piece is an own pawn,
                            // 3) next square is still part of moving direction, 4) next square has opponent pawn

                            if (abs(current_move_direction) == 1 && destination_piece.get_type() == pawn &&
                                opponent_move + 1 < move_length && board[opponent_piece_position + (opponent_move + 1) *
                                                                                                   current_move_direction].get_type() ==
                                                                   pawn &&
                                board[opponent_piece_position + (opponent_move + 1) *
                                                                current_move_direction].get_color() == current_player) {

                                pinned_piece = opponent_piece_position + (opponent_move + 1) * current_move_direction;
                                first_piece_found = true;
                                en_passant_pin_exception = true;
                                opponent_move++;
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
                            (en_passant_pin_exception ? en_passant_pinning : pinning).emplace_back(
                                    opponent_piece_position, current_move_direction, pinned_piece);
                        } else if (destination_piece.get_color() != current_player && !en_passant_pin_exception &&
                                   abs(current_move_direction) == 1 && destination_piece.get_type() == pawn &&
                                   board[opponent_piece_position +
                                         (opponent_move - 1) * current_move_direction].get_type() == pawn &&
                                   board[opponent_piece_position +
                                         (opponent_move - 1) * current_move_direction].get_color() == current_player) {
                            // en passant case with first own piece then opponent piece
                            en_passant_pin_exception = true;
                            continue;
                        } else { break; }
                    }
                }
            }
        } else if (opponent_piece.get_type() == king) {
            int actual_opponent_moves[8];
            int actual_position_counter = 0;
            for (auto opponent_move: sliding_move_directions) {
                if (abs((opponent_move + opponent_piece_position) / 8 - opponent_piece_position / 8) <= 1 &&
                    abs((opponent_move + opponent_piece_position) % 8 - opponent_piece_position % 8) <= 1 &&
                    opponent_move + opponent_piece_position > -1 && opponent_move + opponent_piece_position < 64) {
                    if (abs(king_positions[current_player] - opponent_move - opponent_piece_position) == 1 ||
                        abs(king_positions[current_player] - opponent_move - opponent_piece_position) == 7 ||
                        abs(king_positions[current_player] - opponent_move - opponent_piece_position) == 8 ||
                        abs(king_positions[current_player] - opponent_move - opponent_piece_position) == 9) {
                        defended_squares.insert(opponent_move + opponent_piece_position);
                    }
                    actual_opponent_moves[actual_position_counter] = opponent_piece_position + opponent_move;
                    actual_position_counter++;
                }
            }
            opponent_piece.set_moves(actual_opponent_moves, actual_position_counter);
        } else {
            get_pgn();
            throw invalid_argument("received invalid opponent piece type");
        }
    }


// compute full checking_lines
    vector<vector<int>> checking_lines{checking.size()};
    for (int line = 0; line < checking_lines.size(); line++) {
        for (int move = get<0>(checking.at(line));
             move != king_positions[current_player]; move += get<1>(checking.at(line))) {
            checking_lines.at(line).push_back(move);
        }
    }

// compute own moves depending on if checked or not
    int move_index = 0;
    bool piece_is_pinned = false;
    if (checking_lines.size() > 1) {
        // doublecheck
        for (const int &own_piece: own_pieces) {
            board[own_piece].set_amount_moves(0);
        }
// only king moves need to be done
        int &king_position = king_positions[current_player];
        int move_position = 0;
        Piece &king = board[king_position];
        for (auto direction: sliding_move_directions) {
            if (defended_squares.find(king_position + direction) == defended_squares.end() &&
                (board[king_position + direction].get_type() == none ||
                 board[king_position + direction].get_color() != current_player) && king_position + direction > 0 &&
                king_position + direction < 64) {
                king.set_moves(move_position, king_position + direction);
                move_position++;
            }
        }
        king.set_amount_moves(move_position);
    } else if (!checking_lines.empty()) {
        // check
        for (int own_piece_position: own_pieces) {
            Piece &own_piece = board[own_piece_position];
            piece_is_pinned = false;
            int pin_index = 0;
            move_index = 0;

// see if piece is pinned
            for (auto pin: pinning) {
                if (get<2>(pin) == own_piece_position) {
                    piece_is_pinned = true;
                    break;
                }
                pin_index++;
            }
            if (piece_is_pinned) {
                own_piece.set_amount_moves(0);
                continue;
            }
            if (own_piece.get_type() == pawn) {
                int destination_square = own_piece_position + (current_player ? 8 : -8);
                if (board[destination_square].get_type() == none) {
                    if (find(checking_lines.at(0).begin(), checking_lines.at(0).end(), destination_square) !=
                        checking_lines.at(0).end()) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    } else if (own_piece_position / 8 == (current_player ? 1 : 6)) {
                        destination_square += current_player ? 8 : -8;
                        if (board[destination_square].get_type() == none &&
                            find(checking_lines.at(0).begin(), checking_lines.at(0).end(), destination_square) !=
                            checking_lines.at(0).end()) {
                            own_piece.set_moves(move_index, destination_square);
                            move_index++;
                        }
                    }
                }
                // possibility of taking attacking piece
                update_en_passant();
                for (int moving_direction = 7; moving_direction < 10; moving_direction += 2) {
                    destination_square = own_piece_position + (-1 + 2 * current_player) * moving_direction;
                    if ((destination_square / 8 - own_piece_position / 8 != 1 &&
                        destination_square / 8 - own_piece_position / 8 != -1) || destination_square < 0) { continue; }
                    if (destination_square == (checking_lines.at(0)).at(0)) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    } else if (en_passant_updated_this_move && destination_square == en_passant_square &&
                               (checking_lines.at(0)).at(0) == en_passant_square + (current_player ? -8 : 8)) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == knight) {
                for (auto direction: knight_moving_directions) {
                    int destination_square = own_piece_position + direction;
// check if move blocks check while preserving L-shape, control of 0 <= move < 64 not necessary,
// can be assumed because of reach of pin line
                    if ((destination_square / 8 - own_piece_position / 8 + destination_square % 8 -
                         own_piece_position % 8 > -4 &&
                         destination_square / 8 - own_piece_position / 8 + destination_square % 8 -
                         own_piece_position % 8 < 4) &&
                        find(checking_lines.at(0).begin(), checking_lines.at(0).end(), destination_square) !=
                        checking_lines.at(0).end()) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == bishop || own_piece.get_type() == rook ||
                       own_piece.get_type() == queen) {
                for (int direction_index = 0; direction_index < 8; direction_index++) {
                    const int &move_length = sliding_piece_distances[own_piece.get_type() -
                                                                     3][own_piece_position][direction_index];
                    const int &current_move_direction = sliding_move_directions[direction_index];
                    for (int current_move = 1; current_move <= move_length; current_move++) {
                        int destination_square = own_piece_position + current_move * current_move_direction;
                        if (find(checking_lines.at(0).begin(), checking_lines.at(0).end(), destination_square) !=
                            checking_lines.at(0).end()) {
                            own_piece.set_moves(move_index, destination_square);
                            move_index++;
                            break;
                        } else if (board[destination_square].get_type() != none) {
                            break;
                        }
                    }
                    if (move_index == 3) {
                        break;
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == king) {
                for (auto moving_direction: sliding_move_directions) {
                    int destination_square = own_piece_position + moving_direction;
                    Piece &destination_piece = board[destination_square];
                    if (abs(own_piece_position / 8 - (destination_square) / 8) < 2 &&
                        abs(own_piece_position % 8 - (destination_square) % 8) < 2 && destination_square > -1 &&
                        destination_square < 64 &&
                        (destination_piece.get_type() == none || destination_piece.get_color() != current_player) &&
                        defended_squares.find(destination_square) == defended_squares.end()) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else {
                throw invalid_argument("received invalid own piece type computing check-case");
            }

        }
    } else {

// no check
        for (int own_piece_position: own_pieces) {
            Piece &own_piece = board[own_piece_position];
            piece_is_pinned = false;
            int pin_index = 0;
            move_index = 0;

// see if piece is pinned
            for (auto pin: pinning) {
                if (get<2>(pin) == own_piece_position) {
                    piece_is_pinned = true;
                    break;
                }
                pin_index++;
            }
            if (own_piece.get_type() == pawn) {
                if (piece_is_pinned) {
                    tuple<int, int, int> &current_pin = pinning.at(pin_index);
                    if (abs(get<1>(current_pin)) == 7 || abs(get<1>(current_pin)) == 9) {
// diagonal pin

// normal case
                        bool normal_without_result = true;
                        for (int current_move_direction = 7; current_move_direction < 10; current_move_direction += 2) {
                            int destination_square =
                                    own_piece_position + (current_player ? 1 : -1) * current_move_direction;
                            if (abs(own_piece_position % 8 - destination_square % 8) != 1) {
                                continue;
                            }
                            if (destination_square == get<0>(current_pin)) {
                                own_piece.set_moves(move_index, destination_square);
                                move_index++;
                                normal_without_result = false;
                                break;
                            }
                        }
                        if (normal_without_result) {
// try way too complicated en passant case

// compute en passant square if not already done
                            update_en_passant();
                            if (en_passant_square != -1 && en_passant_updated_this_move) {
                                for (int current_move_direction = 7;
                                     current_move_direction < 10; current_move_direction += 2) {
                                    int destination_square =
                                            own_piece_position + (current_player ? 1 : -1) * current_move_direction;
                                    if (abs(own_piece_position % 8 - destination_square % 8) != 1) {
                                        continue;
                                    }
                                    if (destination_square == en_passant_square &&
                                        (abs(get<1>(current_pin)) == current_move_direction)) {
                                        own_piece.set_moves(move_index, en_passant_square);
                                        move_index++;
                                        break;
                                    }
                                }
                            }
                        }
                    } else if (abs(get<1>(current_pin)) == 8) {
// straight pin vertical
                        if (board[own_piece_position + (current_player ? 8 : -8)].get_type() == none) {
                            own_piece.set_moves(move_index, own_piece_position + (current_player ? 8 : -8));
                            move_index++;
                            if (own_piece_position / 8 == (current_player ? 1 : 6) &&
                                board[own_piece_position + (current_player ? 16 : -16)].get_type() == none) {
                                own_piece.set_moves(move_index, own_piece_position + (current_player ? 16 : -16));
                                move_index++;
                            }
                        }
                    }
                } else {
// no normal pin

// en passant computation
                    update_en_passant();

// diagonal moves
                    for (int diagonal_move = -1; diagonal_move < 2; diagonal_move += 2) {
                        int destination_square = own_piece_position + (current_player ? 8 : -8) + diagonal_move;
                        if ((own_piece_position % 8 - destination_square % 8 != 1 &&
                             own_piece_position % 8 - destination_square % 8 != -1) || destination_square < 0 ||
                            destination_square > 63) {
                            continue;
                        }
                        if (board[destination_square].get_type() != none &&
                            board[destination_square].get_color() != current_player) {
                            own_piece.set_moves(move_index, destination_square);
                            move_index++;
                        } else if (destination_square == en_passant_square && en_passant_updated_this_move) {
                            bool blocked_by_pin = false;
                            for (auto en_passant_pin: en_passant_pinning) {
                                if (get<2>(en_passant_pin) == own_piece_position) {
                                    blocked_by_pin = true;
                                    break;
                                }
                            }
                            if (!blocked_by_pin) {
                                own_piece.set_moves(move_index, destination_square);
                                move_index++;
                            }
                        }
                    }
// straight moves
                    if (board[own_piece_position + (current_player ? 8 : -8)].get_type() == none) {
                        own_piece.set_moves(move_index, own_piece_position + (current_player ? 8 : -8));
                        move_index++;
                        if (own_piece_position / 8 == (current_player ? 1 : 6) &&
                            board[own_piece_position + (current_player ? 16 : -16)].get_type() == none) {
                            own_piece.set_moves(move_index, own_piece_position + (current_player ? 16 : -16));
                            move_index++;
                        }
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == knight) {
                if (piece_is_pinned) {
                    own_piece.set_amount_moves(0);
                    continue;
                }
                for (int moving_direction: knight_moving_directions) {
                    int destination_square = own_piece_position + moving_direction;
// move is not within board (not L-shaped)
                    if (!(destination_square / 8 - own_piece_position / 8 + destination_square % 8 -
                          own_piece_position % 8 > -4 &&
                          destination_square / 8 - own_piece_position / 8 + destination_square % 8 -
                          own_piece_position % 8 < 4) || destination_square < 0 || destination_square > 63) {
                        continue;
                    }
                    Piece &destination_piece = board[destination_square];
                    if (destination_piece.get_type() == none || destination_piece.get_color() != current_player) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == bishop || own_piece.get_type() == rook ||
                       own_piece.get_type() == queen) {

// use positioning of rooks to compute blockage of castling lines
// break move_count == 24
                if (own_piece.get_type() == rook) {
// short
                    int piece_count = 0;
                    if (castling_rights_this_move[0] && castling_rights[2 - current_player * 2] &&
                        own_piece_position == original_castling_rook_positions[1 - current_player][0]) {
                        if (piece_is_pinned) {
                            castling_rights_this_move[0] = false;
                        } else {
                            for (int square = min(king_positions[current_player],
                                                  castling_end_squares[current_player][2]);
                                 square <= max(own_piece_position, castling_end_squares[current_player][0]); square++) {
                                if (board[square].get_type() != none) {
                                    piece_count++;
                                    if (piece_count == 3) {
                                        break;
                                    }
                                }

                            }
                            if (piece_count == 3) {
                                castling_rights_this_move[0] = false;
                            }
                        }
                    } else if (castling_rights_this_move[1] && castling_rights[3 - current_player * 2] &&
                               own_piece_position < king_positions[current_player] &&
                               own_piece_position / 8 == king_positions[current_player] / 8) {
                        if (piece_is_pinned) {
                            castling_rights_this_move[1] = false;
                        } else {
                            for (int square = max(king_positions[current_player],
                                                  castling_end_squares[current_player][3]);
                                 square >= min(own_piece_position, castling_end_squares[current_player][1]); square--) {
                                if (board[square].get_type() != none) {
                                    piece_count++;
                                    if (piece_count == 3) {
                                        break;
                                    }
                                }
                            }
                            if (piece_count == 3) {
                                castling_rights_this_move[1] = false;
                            }
                        }
                    }
                }

// diagonally pinned rooks and straightly pinned bishops never have moves within their pin
                if (piece_is_pinned && (own_piece.get_type() == rook && (abs(get<1>(pinning.at(pin_index))) == 7 ||
                                                                         abs(get<1>(pinning.at(pin_index))) == 9) ||
                                        own_piece.get_type() == bishop && (abs(get<1>(pinning.at(pin_index))) == 1 ||
                                                                           abs(get<1>(pinning.at(pin_index))) == 8))) {
                    own_piece.set_amount_moves(0);
                    continue;
                }

// start actual move update cycle
                short int *sliding_lengths = sliding_piece_distances[own_piece.get_type() - 3][own_piece_position];

// go through every viewing direction

                for (int direction = 0; direction < 8; direction++) {
                    const int &move_length = sliding_lengths[direction];
                    const int &current_move_direction = sliding_move_directions[direction];

// go through the maximum amount of moves in said direction
                    for (int current_move = 1; current_move <= move_length; current_move++) {
                        int destination_square = own_piece_position + current_move * current_move_direction;
                        Piece &destination_piece = board[destination_square];
                        if (piece_is_pinned) {
                            auto &current_pin = pinning.at(pin_index);
                            int max_dist = (king_positions[current_player] - get<0>(current_pin)) / get<1>(current_pin);
                            if ((destination_square - get<0>(current_pin)) % get<1>(current_pin) == 0 &&
                                (get<1>(current_pin) < 0 ? destination_square - get<0>(current_pin) :
                                 -destination_square + get<0>(current_pin)) / get<1>(current_pin) < max_dist &&
                                ((get<1>(current_pin) < 0 && destination_square <= get<0>(current_pin) &&
                                  destination_square > king_positions[current_player]) ||
                                 (get<1>(current_pin) > 0 && destination_square >= get<0>(current_pin) &&
                                  destination_square < king_positions[current_player]))) {

                                own_piece.set_moves(move_index, destination_square);
                                move_index++;
                                if (destination_piece.get_type() != none) {
                                    break;
                                }
                            } else {
                                break;
                            }
                        } else {
                            if (destination_piece.get_type() == none) {
                                own_piece.set_moves(move_index, destination_square);
                                move_index++;
                            } else {
                                if (destination_piece.get_color() != current_player) {
                                    own_piece.set_moves(move_index, destination_square);
                                    move_index++;
                                }
                                break;
                            }
                        }
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == king) {
                for (auto moving_direction: sliding_move_directions) {
                    if (abs(own_piece_position / 8 - (own_piece_position + moving_direction) / 8) < 2 &&
                        abs(own_piece_position % 8 - (own_piece_position + moving_direction) % 8) < 2 &&
                        own_piece_position + moving_direction > -1 && own_piece_position + moving_direction < 64 &&
                        (board[own_piece_position + moving_direction].get_type() == none ||
                         board[own_piece_position + moving_direction].get_color() != current_player) &&
                        defended_squares.find(own_piece_position + moving_direction) == defended_squares.end()) {
                        own_piece.set_moves(move_index, own_piece_position + moving_direction);
                        move_index++;
                    }
                }
                own_piece.set_amount_moves(move_index);

            } else {
                draw();
                throw invalid_argument("received invalid own piece type computing normal update cycle");
            }
        }
// add castling
        if (castling_rights_this_move[0] && castling_rights[2 - current_player * 2]) {
            board[king_positions[current_player]].add_move(king_positions[current_player] + 2);
        }
        if (castling_rights_this_move[1] && castling_rights[3 - current_player * 2]) {
            board[king_positions[current_player]].add_move(king_positions[current_player] - 2);
        }

    }
// deal with game end

// threefold
    if (!perft && positions.size() - undo_count > 5) {
        int found_position = 1;
        stored_position &compare_to = positions.at(positions.size() - 1 - undo_count);
        for (int i = max(0,(int) positions.size()-1-fifty_moves_rule_count); i < positions.size() - 1 - undo_count; i++) {
            stored_position &other = positions.at(i);
            if (compare_to == other) {
                found_position++;
                if (found_position == 3) {
                    return threefold;
                }
            }
        }
    }


// checkmates and stalemate
    bool moves = false;
    for (auto piece: own_pieces) {
        if (board[piece].get_amount_moves() > 0) {
            moves = true;
            break;
        }
    }
    if (moves) {
        return not_over;
    } else if (!checking.empty()) {
        return current_player ? black_win : white_win;
    } else {
        return stalemate;
    }
    return not_over;
}

bool Board::make_move(int starting_square, int destination_square, int promotion_type = 0) {
    Piece &piece = board[starting_square];
    unordered_set<int> &own_pieces = current_player ? white_positions : black_positions;
    unordered_set<int> &opponent_pieces = current_player ? black_positions : white_positions;
    int old_rook_pos = -1;
    int new_rook_pos = -1;
    int piece_taken_pos = destination_square;
    // erase for deleting from set by value
    if (undo_count > 0) {
        move_history.erase(move_history.end() - undo_count, move_history.end());
        positions.erase(positions.end() - undo_count, positions.end());
        undo_count = 0;
    }
    if (piece.get_type() == king && abs(starting_square - destination_square) == 2) {
        // find and set rook
        if (destination_square > starting_square && castling_rights[2 - current_player * 2]) {
            const int &org_rook_square = original_castling_rook_positions[1 - current_player][0];
            board[org_rook_square] = Piece();
            board[castling_end_squares[current_player][2]] = Piece(current_player, rook);
            own_pieces.erase(org_rook_square);
            own_pieces.insert(castling_end_squares[current_player][2]);
            old_rook_pos = org_rook_square;
            new_rook_pos = castling_end_squares[current_player][2];
            destination_square = castling_end_squares[current_player][0];
        } else if (destination_square < starting_square && castling_rights[3 - current_player * 2]) {
            const int &org_rook_square = original_castling_rook_positions[1 - current_player][1];
            board[castling_end_squares[current_player][3]] = board[org_rook_square];
            own_pieces.erase(org_rook_square); // delete rook from own pieces
            board[org_rook_square] = Piece(); // delete rook from board
            own_pieces.insert(castling_end_squares[current_player][3]); // insert rook into own pieces
            old_rook_pos = org_rook_square; // insert rook into board
            new_rook_pos = castling_end_squares[current_player][3]; // set new position of rook
            destination_square = castling_end_squares[current_player][1]; // set correct destination square
        } else { return false; }
    }
    if (piece.get_type() == pawn && destination_square == en_passant_square && en_passant_updated_this_move) {
        piece_taken_pos = starting_square / 8 * 8 + destination_square % 8;
    }
    move_history.push_back(
            stored_move{starting_square, destination_square, piece.get_type(), board[piece_taken_pos].get_type(),
                        piece_taken_pos, en_passant_square,
                        {castling_rights[0], castling_rights[1], castling_rights[2], castling_rights[3]},
                        fifty_moves_rule_count, old_rook_pos, new_rook_pos,
                        convert_piece((piece_types) promotion_type)});

    // update 50 move rule counter if necessary
    if (board[piece_taken_pos].get_type() != none || piece.get_type() == pawn) { fifty_moves_rule_count = 0; }

    // update castling rights
    if (board[starting_square].get_type() == rook) {
        if (starting_square == original_castling_rook_positions[1 - current_player][0]) {
            castling_rights[2 - 2 * current_player] = false;
        } else if (starting_square == original_castling_rook_positions[1 - current_player][1]) {
            castling_rights[3 - 2 * current_player] = false;
        }
    } else if (board[starting_square].get_type() == king) {
        castling_rights[2 - current_player * 2] = false;
        castling_rights[3 - current_player * 2] = false;
        king_positions[current_player] = destination_square;
    }
    if (board[piece_taken_pos].get_type() == rook) {
        // "current_player" correct here since opponent needed
        if (piece_taken_pos == original_castling_rook_positions[current_player][0]) {
            castling_rights[2 * current_player] = false;
        } else if (piece_taken_pos == original_castling_rook_positions[current_player][1]) {
            castling_rights[2 * current_player + 1] = false;
        }
    }

    //handle taken piece
    board[piece_taken_pos] = Piece();
    opponent_pieces.erase(piece_taken_pos);

    // handle promotions
    if (promotion_type < 2 || promotion_type > 5) {
        board[destination_square] = board[starting_square];
    } else {
        board[destination_square] = Piece(current_player, promotion_type);
    }
    // reset original piece square
    board[starting_square] = Piece();

    // update own piece vector
    own_pieces.erase(starting_square);
    own_pieces.insert(destination_square);


    store_current_position();
    current_player = !current_player;
    move_count++;
    fifty_moves_rule_count++;
    en_passant_updated_this_move = false;
    return true;
}

void Board::undo() {
    unordered_set<int> &own_pieces = current_player ? black_positions : white_positions;
    unordered_set<int> &opponent_pieces = current_player ? white_positions : black_positions;
    stored_move *last_played_move = last_move();
    if (last_played_move->previous_square == -1) { return; }
    move_count--;
    undo_count++;
    current_player = !current_player;
    fifty_moves_rule_count = last_played_move->move_rule_count;
    en_passant_square = last_played_move->en_passant_square;
    en_passant_updated_this_move = true;
    for (int i = 0; i < 4; i++) {
        castling_rights[i] = last_played_move->castling_rights[i];
    }
    if (last_played_move->new_rook_square != -1) {
        board[last_played_move->previous_rook_square] = board[last_played_move->new_rook_square];
        board[last_played_move->new_rook_square] = Piece();
        own_pieces.erase(last_played_move->new_rook_square);
        own_pieces.insert(last_played_move->previous_rook_square);
    }
    board[last_played_move->previous_square] = Piece(current_player, last_played_move->piece);
    board[last_played_move->new_square] = Piece();
    if (last_played_move->piece == king) { king_positions[current_player] = last_played_move->previous_square; }
    own_pieces.erase(last_played_move->new_square);
    own_pieces.insert(last_played_move->previous_square);

    if (last_played_move->taken_piece != none) {
        board[last_played_move->piece_taken_at] = Piece(!current_player, last_played_move->taken_piece);
        opponent_pieces.insert(last_played_move->piece_taken_at);
    }


}

void Board::run() {
    while (true) {
        game_ends game_end = generate_piece_moves();
        if (game_end != not_over) {
            draw();
            cout << game_end << " over" << endl;
            break;
        }
        draw();
        string usermove;
        cin >> usermove;
        if (usermove == "undo") {
            cout << "undo" << endl;
            undo();
            continue;
        }
        cout << "move " << (tolower(usermove[0]) - 97) + 8 * (usermove[1] - 49) << " to "
             << (tolower(usermove[2]) - 97) + 8 * (usermove[3] - 49) << " pro "
             << (usermove.size() == 5 ? convert_piece(tolower(usermove[4])) : 0) << endl;
        make_move((tolower(usermove[0]) - 97) + 8 * (usermove[1] - 49),
                  (tolower(usermove[2]) - 97) + 8 * (usermove[3] - 49),
                  usermove.size() == 5 ? convert_piece(tolower(usermove[4])) : 0);
    }
}

int Board::random_games(int count) {
    while (true) {
        //draw();
        //cout << count << " " << current_player << " " << move_count << endl << endl;
        //get_pgn();
        if (generate_piece_moves() != not_over) {
            return move_count;
        }
        unordered_set<int> &own_pieces = current_player ? white_positions : black_positions;
        while (true) {
            auto it = own_pieces.begin();
            int random = rand() % own_pieces.size();
            advance(it, random);
            Piece &chosen_piece = board[*it];
            if (chosen_piece.get_amount_moves() == 0) { continue; }
            int chosen_move = chosen_piece.get_move(rand() % chosen_piece.get_amount_moves());
            if (chosen_piece.get_type() == pawn && chosen_move / 8 == 7 * current_player) {
                make_move(*it, chosen_move, rand() % 4 + 2);
                break;
            } else {
                make_move(*it, chosen_move, 0);
                break;
            }
        }
    }
}

int Board::perft(int depth, map<string, int> *stored, const tuple<int, int, int> &last_moved) {
    int counter = 0;
    int local_amount;
    string local_move;
    if (depth == 0) {
        return 1;
    }
    if (generate_piece_moves(true) != not_over) { return 0; }
    vector<tuple<int, int, int>> moves;
    for (auto piece: (current_player ? white_positions : black_positions)) {
        for (int move_index = 0; move_index < board[piece].get_amount_moves(); move_index++) {
            int destination_square = board[piece].get_move(move_index);
            if (board[piece].get_type() == pawn && (destination_square / 8 == 7 || destination_square / 8 == 0)) {
                for (int i = 2; i < 6; i++) {
                    moves.emplace_back(piece, destination_square, i);
                }
            } else {
                moves.emplace_back(piece, destination_square, 0);
            }
        }
    }
    for (const auto &tested_move: moves) {

        make_move(tested_move);
        //cout << depth << " " << get<0>(tested_move) << " " << get<1>(tested_move) << " " << get<2>(tested_move) << endl;
        local_amount = perft(depth - 1, stored);
        counter += local_amount;
        local_move = string{(char) (get<0>(tested_move) % 8 + 97), (char) (get<0>(tested_move) / 8 + 49),
                            (char) (get<1>(tested_move) % 8 + 97), (char) (get<1>(tested_move) / 8 + 49)};
        switch (get<2>(tested_move)) {
            case 2:
                local_move.push_back('n');
                break;
            case 3:
                local_move.push_back('b');
                break;
            case 4:
                local_move.push_back('r');
                break;
            case 5:
                local_move.push_back('q');
                break;
        }
        //if (depth == 2) {cout << (char) (get<0>(tested_move) % 8 + 97) << get<0>(tested_move) / 8 + 1<< (char) (get<1>(tested_move) % 8 + 97) << get<1>(tested_move) / 8 + 1 << " " << local_amount << endl;}
        if (depth == 5) { (*stored)[local_move] = local_amount; }
        //cout << "undo " << depth  << " " << get<0>(tested_move) << " " << get<1>(tested_move) << " " << get<2>(tested_move) << endl;
        undo();
    }
    return counter;
}

void Board::draw() {
    char output[64];
    for (char &i: output) i = '#';
    for (int i = 0; i < 64; i++) {
        convert_piece(board[i]);
        output[(7 - i / 8) * 8 + i % 8] = convert_piece(board[i]);
    }
    for (int i = 0; i < 64; i++) {
        cout << output[i] << " ";
        if (i % 8 == 7) { cout << endl; }
    }
    cout << endl;
}

void Board::store_current_position() {
    int castles = 0;
    array<int, 64> position{};
    for (int i = 0; i < 64; i++) {
        position[i] = board[i].get_color() * 6 + board[i].get_type();
    }
    for (int i = 0; i < 4; i++) {
        castles += castling_rights[i] * (int) pow(2, i);
    }
    positions.push_back(stored_position{position, en_passant_square, castles});
}

void Board::get_pgn() {
    string pgn;
    for (stored_move &current_move: move_history) {
        if (current_move.previous_rook_square != -1) {
            if (current_move.new_rook_square > current_move.new_square) {
                pgn += "O-O-O ";
                continue;
            } else {
                pgn += "O-O ";
                continue;
            }
        }
        if (current_move.piece != pawn) { pgn.push_back(convert_piece(current_move.piece)); }
        pgn.push_back((char) (current_move.previous_square % 8 + 97));
        pgn.push_back((char) (current_move.previous_square / 8 + 49));
        pgn.push_back((char) (current_move.new_square % 8 + 97));
        pgn.push_back((char) (current_move.new_square / 8 + 49));
        if (current_move.promotion_type != 'P' && current_move.promotion_type != ' ' &&
            current_move.promotion_type != 'K') {
            pgn.push_back(current_move.promotion_type);
        }
        pgn.push_back(' ');
    }
    cout << pgn << endl;
}

void Board::update_en_passant() {
    if (!en_passant_updated_this_move) {
        en_passant_updated_this_move = true;
        stored_move *last = last_move();
        if (last->piece == pawn && abs(last->previous_square - last->new_square) == 16) {
            en_passant_square = last->previous_square + (current_player ? -8 : 8);
        } else { en_passant_square = -1; }
    }
}