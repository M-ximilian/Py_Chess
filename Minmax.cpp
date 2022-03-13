//
// Created by maxim on 07.03.2022.
//

#include "Minmax.h"

float piece_values[6] = {10, 30, 32, 50, 90, 0};
static const float eval_positions[12][64] = {{0,  0,  0,  0,    0,    0,  0,  0,  -5,   -5,  -5, -5,  -5,  -5, -5,  -5,   -1,   -1,   -2,   -3,   -3,   -2,   -1,   -1,   -0.5, -0.5, -1,   -2.5, -2.5, -1,   -0.5, -0.5, 0,    0,   0,    -2,   -2,   0,    0,   0,    -0.5, 0.5,  1,   0,    0,    1,   0.5,  -0.5, -0.5, -1,   -1, 2,    2,    -1, -1,   -0.5, 0,  0,  0,  0,    0,    0,  0,  0}, // p
                                             {5,  4,  3,  3,    3,    3,  4,  5,  4,    2,   0,  0,   0,   0,  2,   4,    3,    0,    -1,   -1.5, -1.5, -1,   0,    3,    3,    -0.5, -1.5, -2,   -2,   -1.5, -0.5, 3,    3,    0,   -1.5, -2,   -2,   -1.5, 0,   3,    3,    -0.5, -1,  -1.5, -1.5, -1,  -0.5, 3,    4,    2,    0,  -0.5, -0.5, 0,  2,    4,    5,  4,  3,  3,    3,    3,  4,  5}, // n
                                             {2,  1,  1,  1,    1,    1,  1,  2,  1,    0,   0,  0,   0,   0,  0,   1,    1,    0,    -0.5, -1,   -1,   -0.5, 0,    1,    1,    -0.5, -0.5, -1,   -1,   -0.5, -0.5, 1,    1,    0,   -1,   -1,   -1,   -1,   0,   1,    1,    -1,   -1,  -1,   -1,   -1,  -1,   1,    1,    -0.5, 0,  0,    0,    0,  -0.5, 1,    2,  1,  1,  1,    1,    1,  1,  2}, // b
                                             {0,  0,  0,  0,    0,    0,  0,  0,  -0.5, -1,  -1, -1,  -1,  -1, -1,  -0.5, 0.5,  0,    0,    0,    0,    0,    0,    0.5,  0.5,  0,    0,    0,    0,    0,    0,    0.5,  0.5,  0,   0,    0,    0,    0,    0,   0.5,  0.5,  0,    0,   0,    0,    0,   0,    0.5,  0.5,  0,    0,  0,    0,    0,  0,    0.5,  0,  0,  0,  -0.5, -0.5, 0,  0,  0}, // r
                                             {2,  1,  1,  0.5,  0.5,  1,  1,  2,  1,    0,   0,  0,   0,   0,  0,   1,    1,    0,    -0.5, -0.5, -0.5, -0.5, 0,    1,    0,    0,    -0.5, -0.5, -0.5, -0.5, 0,    0,    0,    0,   -0.5, -0.5, -0.5, -0.5, 0,   0,    1,    0,    0,   -0.5, -0.5, 0,   0,    1,    1,    0,    0,  0,    0,    0,  0,    1,    2,  1,  1,  0.5,  0.5,  1,  1,  2}, // q
                                             {3,  4,  4,  5,    5,    4,  4,  3,  3,    4,   4,  5,   5,   4,  4,   3,    3,    4,    4,    5,    5,    4,    4,    3,    3,    4,    4,    5,    5,    4,    4,    3,    2,    3,   3,    4,    4,    3,    3,   2,    1,    2,    2,   2,    2,    2,   2,    1,    -2,   -2,   0,  0,    0,    0,  -2,   -2,   -2, -3, -1, 0,    0,    -1, -3, -2}, // k
                                             {0,  0,  0,  0,    0,    0,  0,  0,  0.5,  1,   1,  -2,  -2,  1,  1,   0.5,  0.5,  -0.5, -1,   0,    0,    -1,   -0.5, 0.5,  0,    0,    0,    2,    2,    0,    0,    0,    0.5,  0.5, 1,    2.5,  2.5,  1,    0.5, 0.5,  1,    1,    2,   3,    3,    2,   1,    1,    5,    5,    5,  5,    5,    5,  5,    5,    0,  0,  0,  0,    0,    0,  0,  0}, // P
                                             {-5, -4, -3, -3,   -3,   -3, -4, -5, -4,   -2,  0,  0.5, 0.5, 0,  -2,  -4,   -3,   0.5,  1,    1.5,  1.5,  1,    0.5,  -3,   -3,   0,    1.5,  2,    2,    1.5,  0,    -3,   -3,   0.5, 1.5,  2,    2,    1.5,  0.5, -3,   -3,   0,    1,   1.5,  1.5,  1,   0,    -3,   -4,   -2,   0,  0,    0,    0,  -2,   -4,   -5, -4, -3, -3,   -3,   -3, -4, -5}, // N
                                             {-2, -1, -1, -1,   -1,   -1, -1, -2, -1,   0.5, 0,  0,   0,   0,  0.5, -1,   -1,   1,    1,    1,    1,    1,    1,    -1,   -1,   0,    1,    1,    1,    1,    0,    -1,   -1,   0.5, 0.5,  1,    1,    0.5,  0.5, -1,   -1,   0,    0.5, 1,    1,    0.5, 0,    -1,   -1,   0,    0,  0,    0,    0,  0,    -1,   -2, -1, -1, -1,   -1,   -1, -1, -2}, // B
                                             {0,  0,  0,  0.5,  0.5,  0,  0,  0,  -0.5, 0,   0,  0,   0,   0,  0,   -0.5, -0.5, 0,    0,    0,    0,    0,    0,    -0.5, -0.5, 0,    0,    0,    0,    0,    0,    -0.5, -0.5, 0,   0,    0,    0,    0,    0,   -0.5, -0.5, 0,    0,   0,    0,    0,   0,    -0.5, 0.5,  1,    1,  1,    1,    1,  1,    0.5,  0,  0,  0,  0,    0,    0,  0,  0}, // R
                                             {-2, -1, -1, -0.5, -0.5, -1, -1, -2, -1,   0,   0,  0,   0,   0,  0,   -1,   -1,   0,    0,    0.5,  0.5,  0,    0,    -1,   0,    0,    0.5,  0.5,  0.5,  0.5,  0,    0,    0,    0,   0.5,  0.5,  0.5,  0.5,  0,   0,    -1,   0,    0.5, 0.5,  0.5,  0.5, 0,    -1,   -1,   0,    0,  0,    0,    0,  0,    -1,   -2, -1, -1, -0.5, -0.5, -1, -1, -2}, // Q
                                             {2,  3,  1,  0,    0,    1,  3,  2,  2,    2,   0,  0,   0,   0,  2,   2,    -1,   -2,   -2,   -2,   -2,   -2,   -2,   -1,   -2,   -3,   -3,   -4,   -4,   -3,   -3,   -2,   -3,   -4,  -4,   -5,   -5,   -4,   -4,  -3,   -3,   -4,   -4,  -5,   -5,   -4,  -4,   -3,   -3,   -4,   -4, -5,   -5,   -4, -4,   -3,   -3, -4, -4, -5,   -5,   -4, -4, -3}}; // K

Minmax::Minmax(Board *g) {
    game = g;
}

float Minmax::minmax(int storage_position, int depth, int current_depth, float alpha, float beta) {
    if (current_depth == 0) {
        node_count++;
        //if (node_count % 10000 == 0) {cout << node_count << " nodes rn" << endl;}
        return evaluate();
    } else if (current_depth == -1) {
        current_depth = depth;
    }
    // eventual move_sorting here
    vector<tuple<int, int, int>> moves = order_moves();


    float max_score = -10000000;
    float min_score = 10000000;
    float eval;


    // simulate all moves of current depth
    for (const auto &current_move: moves) {
        // do move
        game->make_move(current_move);

        // update moves
        switch (game->generate_piece_moves()) {
            case not_over:
                break;
            case white_win:
                game->undo();
                if (current_depth == depth && (best_moves_per_iteration[depth].empty() || !game->current_player &&
                                                                                          (100000 - (float) (depth -
                                                                                                             current_depth +
                                                                                                             1)) <
                                                                                          min_score ||
                                               game->current_player &&
                                               (100000 - (float) (depth - current_depth + 1)) > max_score)) {
                    best_moves_per_iteration[storage_position] = {current_move};
                } else if (current_depth == depth &&
                           (!game->current_player && (100000 - (float) (depth - current_depth + 1)) == min_score ||
                            game->current_player && (100000 - (float) (depth - current_depth + 1)) == max_score)) {
                    best_moves_per_iteration[storage_position].push_back(current_move);
                }
                return (100000) - (float) (depth - current_depth + 1);

            case black_win:
                game->undo();
                if (current_depth == depth && (best_moves_per_iteration[depth].empty() || !game->current_player &&
                                                                                          (-100000 + (float) (depth -
                                                                                                              current_depth +
                                                                                                              1)) <
                                                                                          min_score ||
                                               game->current_player &&
                                               (-100000 + (float) (depth - current_depth + 1)) > max_score)) {
                    best_moves_per_iteration[storage_position] = {current_move};
                } else if (current_depth == depth &&
                           (!game->current_player && (-100000 + (float) (depth - current_depth + 1)) == min_score ||
                            game->current_player && (-100000 + (float) (depth - current_depth + 1)) == max_score)) {
                    best_moves_per_iteration[storage_position].push_back(current_move);
                }
                return (-100000) + (float) (depth - current_depth + 1);

            default: // draw
                game->undo();
                if (current_depth == depth &&
                    (best_moves_per_iteration[depth].empty() || !game->current_player && 0 < min_score ||
                     game->current_player && 0 > max_score)) {
                    best_moves_per_iteration[storage_position] = {current_move};
                } else if (current_depth == depth &&
                           (!game->current_player && 0 == min_score || game->current_player && 0 == max_score)) {
                    best_moves_per_iteration[storage_position].push_back(current_move);
                }
                return 0;
        }
        eval = minmax(storage_position, depth, current_depth - 1, alpha, beta);
        game->undo();

        // evaluate the results depending on color
        if (game->current_player) {
            // adding best moves stuff
            if (eval > max_score) {
                max_score = eval;
                // pruning stuff
                alpha = max(alpha, eval);
                if (current_depth == depth) { best_moves_per_iteration[storage_position] = {current_move}; }
            } else if (eval == max_score && current_depth == depth) {
                best_moves_per_iteration[storage_position].push_back(current_move);
            }


        } else {
            // adding best moves stuff
            if (eval < min_score) {
                min_score = eval;
                // pruning stuff
                beta = min(beta, eval);
                if (current_depth == depth) {
                    best_moves_per_iteration[storage_position] = {current_move};
                }
            } else if (eval == min_score && current_depth == depth) {
                best_moves_per_iteration[storage_position].push_back(current_move);
            }


        }
        if (beta <= alpha) { break; }
    }
    if (game->move_history.back().previous_square == 5) {}


    return (game->current_player ? max_score : min_score);
}

float Minmax::evaluate() {
    float eval = 0;
    for (auto &i: game->white_positions) {
        eval += piece_values[game->board[i].get_type() - 1] + eval_positions[6+(game->board[i].get_type())-1][i];
    }
    for (auto &i: game->black_positions) {
        eval -= piece_values[game->board[i].get_type() - 1];
        eval += eval_positions[(game->board[i].get_type())-1][i];
    }
    return eval;
}

tuple<int, int, int> Minmax::get_move(bool use_depth, int depth_or_time) {
    if (use_depth) {
        best_moves_per_iteration.clear();
        node_count = 0;
        best_moves_per_iteration.emplace_back();
        //game->generate_piece_moves();
        float eval = minmax(0, depth_or_time);
        for (auto &bestmove: best_moves_per_iteration[0]) {
            //cout << get<0>(bestmove) << " " << get<1>(bestmove) << endl;
        }
        cout << node_count << " nodes this move " << eval << endl;
        return best_moves_per_iteration[0].at(rand() % best_moves_per_iteration[0].size());
    } else {
        return {};
    };
}

vector<tuple<int, int, int>> Minmax::order_moves() {
    vector<tuple<int, int, int>> ordered_moves;
    vector<float> scores;
    bool &color = game->current_player;
    for (int i:(game->current_player?game->white_positions:game->black_positions)) {
        Piece &piece = game->board[i];
        for (int move_index = 0; move_index < piece.get_amount_moves(); move_index++) {

            if (piece.get_type() == pawn && (piece.get_move(move_index)/8 == 7 || piece.get_move(move_index)/8 == 0)) {
                for (int promotion = 2; promotion < 6; promotion++) {
                    ordered_moves.emplace_back(i, piece.get_move(move_index), promotion);
                    if (game->board[piece.get_move(move_index)].get_type() != none) {
                        if (color) {
                            scores.push_back(piece_values[promotion-1] - piece_values[0] + piece_values[game->board[piece.get_move(move_index)].get_type()-1]);
                        } else {
                            scores.push_back(-piece_values[promotion-1] + piece_values[0] - piece_values[game->board[piece.get_move(move_index)].get_type()-1]);
                        }
                    } else {
                        if (color) {
                            scores.push_back(piece_values[promotion-1]-piece_values[0]);
                        } else {
                            scores.push_back(-piece_values[promotion-1]+piece_values[0]);
                        }
                    }
                }

            } else {
                ordered_moves.emplace_back(i, piece.get_move(move_index), 0);
                int destination = piece.get_move(move_index);
                if (game->board[destination].get_type() != none) {
                    if (color) {
                        scores.push_back(piece_values[game->board[destination].get_type()-1]-piece_values[piece.get_type()-1]);
                    } else {
                        scores.push_back(piece_values[-game->board[destination].get_type()-1]+piece_values[piece.get_type()-1]);
                    }
                } else if (piece.get_color() && destination/8 != 7 && ((destination%8!=0 && game->board[destination+7].get_type() == pawn && !game->board[destination+7].get_color()) || destination%8!=7 && game->board[destination+9].get_type() == pawn && !game->board[destination+9].get_color())) {
                    //expecting to lose the piece without compensation
                    if (color) {
                        scores.push_back(-piece_values[game->board[destination].get_type()-1]);
                    } else {
                        scores.push_back(piece_values[game->board[destination].get_type()-1]);
                    }
                } else if (!piece.get_color() && destination/8 != 0 && ((destination%8!=7 && game->board[destination-7].get_type() == pawn && game->board[destination-7].get_color()) || destination%8!=0 && game->board[destination-9].get_type() == pawn && game->board[destination-9].get_color())) {
                    //expecting to lose the piece without compensation
                    if (color) {
                        scores.push_back(-piece_values[game->board[destination].get_type()-1]);
                    } else {
                        scores.push_back(piece_values[game->board[destination].get_type()-1]);
                    }
                } else {
                    scores.push_back(0);
                }
            }
        }
    }
    return sort_moves(ordered_moves, scores);
}

vector<tuple<int, int, int>> Minmax::sort_moves(vector<tuple<int, int, int>> &moves, vector<float> &scores) {
    int swap;
    for (int i = 0; i < moves.size()-1;i++) {
        for (int j = i + 1; j > 0; j--) {
            swap = j-1;
            if (scores.at(swap) < scores.at(j)) {
                iter_swap(moves.begin()+j, moves.begin()+swap);
                iter_swap(scores.begin()+j, scores.begin()+swap);
            }
        }
    }
    return moves;
}
