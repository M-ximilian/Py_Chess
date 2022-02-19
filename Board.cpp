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
            if (piece.get_color()) { white_positions.insert(i); }
            else { black_positions.insert(i); }
        }
        i++;
    }
    current_player = get<0>(converted_fen);
    en_passant_square = get<1>(converted_fen);
    if (en_passant_square != -1) { en_passant_updated_this_move = true; }
    fifty_moves_rule_count = get<2>(converted_fen);
    move_count = get<3>(converted_fen);
    store_current_position();
}

game_ends Board::generate_piece_moves() {
    auto time_before = chrono::high_resolution_clock::now();
    // compute available pre update game end szenarios
    if (fifty_moves_rule_count > 99) { return fifty_move_rule; }
    int count_pieces = 0;
    bool enough_material = false;
    int bishops[2] = {0, 0};
    int knights[2] = {0, 0};
    for (auto piece_index: white_positions) {
        Piece &piece = board[piece_index];
        if (piece.get_type() == pawn || piece.get_type() == rook || piece.get_type() == queen) {
            enough_material = true;
            break;
        }
        if (piece.get_type() == bishop) { bishops[(piece_index / 8 % 2 + piece_index % 2) % 2]++; }
        else if (piece.get_type() == knight) { knights[1]++; }
        count_pieces++;
    }
    if (!enough_material) {
        for (auto piece_index: black_positions) {
            Piece &piece = board[piece_index];
            if (piece.get_type() == pawn || piece.get_type() == rook || piece.get_type() == queen) {
                enough_material = true;
                break;
            }
            if (piece.get_type() == bishop) { bishops[(piece_index / 8 % 2 + piece_index % 2) % 2]++; }
            else if (piece.get_type() == knight && knights[1] != 0) {
                enough_material = true;
                break;
            } else if (piece.get_type() == knight && knights[1] == 0) { knights[0]++; }
            count_pieces++;
        }
    }
    if (!enough_material && (count_pieces < 4 || count_pieces == 4 && (knights[0] == 0 && knights[1] == 2 ||
                                                                       knights[1] == 0 && knights[0] == 2) ||
                             knights[0] + knights[1] == 0 && (bishops[0] == 0 || bishops[1] == 0))) {
        return insufficient_material;
    }


    checking.clear();
    pinning.clear();
    en_passant_pinning.clear(), defended_squares.clear();
    if (!en_passant_from_fen) { en_passant_updated_this_move = false; }
    else { en_passant_from_fen = false; }
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
            for (auto destination_square: {7 - 16 * current_player, 9 - 16 * current_player}) {
                if (abs(opponent_piece_position % 8 - destination_square % 8) != 1) { continue; }
                // disable castling if opponent sees part of king castling route
                // short
                if (castling_rights[2 * current_player] && castling_rights_this_move[0]) {
                    if (destination_square <= castling_end_squares[current_player][0] &&
                        destination_square > king_positions[current_player]) {
                        castling_rights_this_move[0] = false;
                    }
                    //long
                } else if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1] &&
                           opponent_piece.get_color() != current_player) {
                    if (destination_square >= castling_end_squares[current_player][1] &&
                        destination_square < king_positions[current_player]) {
                        castling_rights_this_move[1] = false;
                    }
                } else if (destination_square == king_positions[current_player]) {
                    checking.emplace_back(opponent_piece_position, destination_square);
                }
                if (abs(king_positions[current_player] - destination_square - opponent_piece_position) == 1 ||
                    abs(king_positions[current_player] - destination_square - opponent_piece_position) == 7 ||
                    abs(king_positions[current_player] - destination_square - opponent_piece_position) == 8 ||
                    abs(king_positions[current_player] - destination_square - opponent_piece_position) == 9) {
                    defended_squares.insert(destination_square + opponent_piece_position);
                }
            }
        } else if (opponent_piece.get_type() == knight) {

            for (auto direction: knight_moving_directions) {
                int destination_square = opponent_piece_position + direction;
                Piece &destination_piece = board[destination_square];

                // skip moves that would leave board
                if (!(abs(destination_square / 8 - opponent_piece_position / 8) == 2 &&
                      abs(destination_square % 8 - opponent_piece_position % 8) == 1 ||
                      (abs(destination_square / 8 - opponent_piece_position / 8) == 1 &&
                       abs(destination_square % 8 - opponent_piece_position % 8) == 2)) || destination_square < 0 ||
                    destination_square > 63) {
                    continue;
                }

                // disable castling if opponent sees part of king castling route
                //short
                if (castling_rights[2 * current_player] && castling_rights_this_move[0]) {
                    if (destination_square <= castling_end_squares[current_player][0] &&
                        destination_square > king_positions[current_player]) {
                        castling_rights_this_move[0] = false;
                    }
                    //long
                } else if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1]) {
                    if (destination_square >= castling_end_squares[current_player][1] &&
                        destination_square < king_positions[current_player]) {
                        castling_rights_this_move[1] = false;
                    }
                } else if (destination_square == king_positions[current_player]) {
                    checking.emplace_back(opponent_piece_position, destination_square);
                    castling_rights_this_move[0] = false;
                    castling_rights_this_move[1] = false;
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
                        // short
                        if (castling_rights[2 * current_player] && castling_rights_this_move[0]) {
                            if (destination_square <= castling_end_squares[current_player][0] &&
                                destination_square > king_positions[current_player]) {
                                castling_rights_this_move[0] = false;
                            }
                            //long
                        } else if (castling_rights[2 * current_player + 1] && castling_rights_this_move[1] &&
                                   opponent_piece.get_color() != current_player) {
                            if (destination_square >= castling_end_squares[current_player][1] &&
                                destination_square < king_positions[current_player]) {
                                castling_rights_this_move[1] = false;
                            }
                        } else if (destination_square == king_positions[current_player]) {
                            checking.emplace_back(opponent_piece_position, current_move_direction);
                            defended_squares.insert(king_positions[current_player] + current_move_direction);
                            break;
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
                                opponent_move != move_length && board[opponent_piece_position + (opponent_move + 1) *
                                                                                                current_move_direction].get_type() ==
                                                                pawn && board[opponent_piece_position +
                                                                              (opponent_move + 1) *
                                                                              current_move_direction].get_color() ==
                                                                        current_player) {
                                pinned_piece = opponent_piece_position + (opponent_move + 1) * current_move_direction;
                                first_piece_found = true;
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
        } else { throw invalid_argument("received invalid opponent piece type"); }
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
    int en_passant_pawn_counter = 0;
    int move_index = 0;
    bool piece_is_pinned = false;
    if (checking_lines.size() > 1) {
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
                 board[king_position + direction].get_color() != current_player)) {
                king.set_moves(move_position, king_position + direction);
                move_position++;
            }
        }
        king.set_amount_moves(move_position);
    } else if (!checking_lines.empty()) {
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
                // en passant that blockes check not possible in normal game, only if given via fen
                if (en_passant_square != -1 && en_passant_updated_this_move && en_passant_pawn_counter < 2 &&
                    abs(en_passant_square - own_piece_position + (current_player ? 8 : -8)) == 1 &&
                    find(checking_lines.at(0).begin(), checking_lines.at(0).end(), en_passant_square) !=
                    checking_lines.at(0).end()) {
                    own_piece.set_moves(move_index, en_passant_square);
                    en_passant_pawn_counter++;
                    move_index++;
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == knight) {
                for (auto direction: knight_moving_directions) {
                    int destination_square = own_piece_position + direction;
                    if (find(checking_lines.at(0).begin(), checking_lines.at(0).end(), destination_square) !=
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

                        }
                    }
                    if (move_index == 3) { break; }
                }
                own_piece.set_amount_moves(move_index);
            } else if (own_piece.get_type() == king) {
                for (auto moving_direction: sliding_move_directions) {
                    int destination_square = own_piece_position + moving_direction;
                    Piece &destination_piece = board[destination_square];
                    if (abs(own_piece_position / 8 - (destination_square) / 8) < 2 &&
                        abs(own_piece_position % 8 - (destination_square) % 8) < 2 &&
                        destination_square > -1 && destination_square < 64 &&
                        (destination_piece.get_type() == none || destination_piece.get_color() != current_player) &&
                        defended_squares.find(destination_square) == defended_squares.end()) {
                        own_piece.set_moves(move_index, destination_square);
                        move_index++;
                    }
                }
                own_piece.set_amount_moves(move_index);
            } else { throw invalid_argument("received invalid own piece type computing check-case"); }

        }
    } else {

        // no check
        for (int own_piece_position: own_pieces) {
            Piece &own_piece = board[own_piece_position];
            pinned_piece = false;
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
                            if (abs(own_piece_position % 8 - destination_square % 8) != 1) { continue; }
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
                            if (en_passant_square == -1 && !en_passant_updated_this_move) {
                                en_passant_updated_this_move = true;
                                stored_move *last = last_move();
                                if (last->piece == pawn && last->previews_square / 8 == (current_player ? 6 : 1) &&
                                    abs(last->previews_square - last->new_square) == 16) {
                                    en_passant_square = last->previews_square + (current_player ? -8 : 8);
                                }
                            }
                            if (en_passant_square != -1 && en_passant_pawn_counter < 2) {
                                int maximum_pin_distance =
                                        (king_positions[current_player] - get<0>(current_pin)) / get<1>(current_pin);
                                for (int current_move_direction = 7;
                                     current_move_direction < 10; current_move_direction += 2) {
                                    int destination_square =
                                            own_piece_position + (current_player ? 1 : -1) * current_move_direction;
                                    if (abs(own_piece_position % 8 - destination_square % 8) != 1) { continue; }
                                    if (destination_square == en_passant_square &&
                                        abs((destination_square - get<0>(current_pin)) / get<1>(current_pin)) <
                                        abs(maximum_pin_distance) &&
                                        (destination_square - get<0>(current_pin)) * maximum_pin_distance *
                                        get<1>(current_pin) > 0) {
                                        own_piece.set_moves(move_index, destination_square);
                                        move_index++;
                                        en_passant_pawn_counter++;
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
                    if (en_passant_square == -1 && !en_passant_updated_this_move) {
                        en_passant_updated_this_move = true;
                        stored_move *last = last_move();
                        if (last->piece == pawn && last->previews_square / 8 == (current_player ? 6 : 1) &&
                            abs(last->previews_square - last->new_square) == 16) {
                            en_passant_square = last->previews_square + (current_player ? -8 : 8);
                        }
                    }

                    // diagonal moves
                    for (int diagonal_move = -1; diagonal_move < 2; diagonal_move += 2) {
                        int destination_square = own_piece_position + (current_player ? 8 : -8) + diagonal_move;
                        if (abs(own_piece_position % 8 - destination_square % 8) != 1) { continue; }
                        if (board[destination_square].get_type() != none &&
                            board[destination_square].get_color() != current_player) {
                            own_piece.set_moves(move_index, destination_square);
                            move_index++;
                        } else if (destination_square == en_passant_square) {
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
                    if (abs(destination_square / 8 - own_piece_position / 8) +
                        abs(destination_square % 8 - own_piece_position % 8) != 3 || destination_square < 0 ||
                        destination_square > 63) {
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
                if (own_piece.get_type() == rook) {
                    // short
                    int piece_count = 0;
                    if (castling_rights_this_move[0] && castling_rights[current_player * 2] &&
                        own_piece_position > king_positions[current_player] &&
                        own_piece_position / 8 == king_positions[current_player] / 8) {
                        if (piece_is_pinned) { castling_rights_this_move[0] = false; }
                        else {
                            for (int square = min(king_positions[current_player],
                                                  castling_end_squares[current_player][2]);
                                 square <= max(own_piece_position, castling_end_squares[current_player][0]); square++) {
                                if (board[square].get_type() != none) {
                                    piece_count++;
                                    if (piece_count == 3) { break; }
                                }

                            }
                            if (piece_count == 3) { castling_rights_this_move[0] = false; }
                        }
                    } else if (castling_rights_this_move[1] && castling_rights[current_player * 2 + 1] &&
                               own_piece_position < king_positions[current_player] &&
                               own_piece_position / 8 == king_positions[current_player] / 8) {
                        if (piece_is_pinned) { castling_rights_this_move[1] = false; }
                        else {
                            for (int square = max(king_positions[current_player],
                                                  castling_end_squares[current_player][3]);
                                 square >= min(own_piece_position, castling_end_squares[current_player][1]); square--) {
                                if (board[square].get_type() != none) {
                                    piece_count++;
                                    if (piece_count == 3) { break; }
                                }
                            }
                            if (piece_count == 3) { castling_rights_this_move[1] = false; }
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
                            if (destination_square % get<1>(current_pin) == get<0>(current_pin) % get<1>(current_pin) &&
                                abs((destination_square - get<0>(current_pin)) / get<1>(current_pin)) < max_dist) {
                                own_piece.set_moves(move_index, destination_square);
                                move_index++;
                                if (destination_piece.get_type() != none) {
                                    break;
                                }
                            } else { break; }
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

            } else { throw invalid_argument("received invalid own piece type computing normal update cycle"); }
        }
        // add castling
        if (castling_rights_this_move[0] && castling_rights[current_player * 2]) {
            board[king_positions[current_player]].add_move(king_positions[current_player] + 2);
        }
        if (castling_rights_this_move[1] && castling_rights[current_player * 2 + 1]) {
            board[king_positions[current_player]].add_move(king_positions[current_player] - 2);
        }

    }
    // deal with game end
    // threefold
    if (positions.size() - undo_count > 5) {
        int found_position = 1;
        stored_position &compare_to = positions.at(positions.size() - 1 - undo_count);
        for (int i = 0; i < positions.size() - 1 - undo_count; i++) {
            stored_position &other = positions.at(i);
            if (compare_to == other) {
                found_position++;
                if (found_position == 3) { return threefold; }
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
    if (moves) { return not_over; }
    else if (!checking.empty()) { return current_player ? black_win : white_win; }
    else { return stalemate; }

    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    //cout << "Time needed: " << time / std::chrono::nanoseconds(1) << endl;
    return not_over;
}

bool Board::make_move(int starting_square, int destination_square, int promotion_type = 0) {
    Piece &piece = board[starting_square];
    unordered_set<int> &own_pieces = current_player ? white_positions : black_positions;
    unordered_set<int> &opponent_pieces = current_player?black_positions:white_positions;
    int old_rook_pos = -1;
    int new_rook_pos = -1;
    int piece_taken_pos = destination_square;
    // erase for deleting from set by value
    if (undo_count > 0) {
        move_history.erase(move_history.end() - 1 - undo_count, move_history.end());
        positions.erase(positions.end() - 1 - undo_count, positions.end());
        undo_count = 0;
    }
    if (piece.get_type() == king && abs(starting_square - destination_square) == 2) {
        // find and set rook
        if (destination_square > starting_square && castling_rights[current_player * 2]) {
            for (int i = starting_square + 1; i < starting_square / 8 + 8; i++) {
                if (board[i].get_type() == rook) {
                    board[castling_end_squares[current_player][2]] = board[i];
                    own_pieces.erase(i);
                    own_pieces.insert(castling_end_squares[current_player][2]);
                    old_rook_pos = i;
                    new_rook_pos = castling_end_squares[current_player][2];
                    destination_square = castling_end_squares[current_player][0];
                }
            }
        } else if (destination_square < starting_square && castling_rights[current_player * 2 + 1]) {
            for (int i = starting_square - 1; i > starting_square / 8 - 1; i--) {
                if (board[i].get_type() == rook) {
                    board[castling_end_squares[current_player][3]] = board[i];
                    own_pieces.erase(i);
                    own_pieces.insert(castling_end_squares[current_player][3]);
                    old_rook_pos = i;
                    new_rook_pos = castling_end_squares[current_player][3];
                    destination_square = castling_end_squares[current_player][1];
                }
            }
        } else { return false; }
    }
    if (piece.get_type() == pawn && destination_square == en_passant_square) {
        piece_taken_pos = starting_square / 8 * 8 + destination_square % 8;
    }
    move_history.push_back(
            stored_move{starting_square, destination_square, piece.get_type(), board[piece_taken_pos].get_type(),
                        piece_taken_pos, en_passant_square,
                        {castling_rights[0], castling_rights[1], castling_rights[2], castling_rights[3]}, old_rook_pos,
                        new_rook_pos});

    if (board[piece_taken_pos].get_type() != none) {amount_of_pieces--;}

    board[piece_taken_pos] = Piece();
    opponent_pieces.erase(piece_taken_pos);
    if (promotion_type < 2 || promotion_type > 5) {
        board[destination_square] = board[starting_square];
    } else {
        board[destination_square] = Piece(current_player, promotion_type);
    }
    board[starting_square] = Piece();

    own_pieces.erase(starting_square);
    own_pieces.insert(destination_square);

    if (board[destination_square].get_type() == rook) {
        castling_rights[current_player*2+(destination_square > king_positions[current_player])] = false;
    }
    else if (board[destination_square].get_type() == king) {
        castling_rights[0] = false;
        castling_rights[1] = false;
        king_positions[current_player] = destination_square;
    }


    store_current_position();
    return true;
}
void Board::undo() {

}

void Board::store_current_position() {
    double position = 0;
    int castles = 0;
    for (auto i: white_positions) {
        position += (board[i].get_color() * 6 + board[i].get_type()) * pow(13, i);
    }
    for (auto i: black_positions) {
        position += (board[i].get_color() * 6 + board[i].get_type()) * pow(13, i);
    }
    for (int i = 0; i < 4; i++) {
        castles += castling_rights[i] * (int) pow(2, i);
    }
    positions.push_back(stored_position{position, en_passant_square, castles});
}