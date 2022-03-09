//
// Created by maxim on 07.03.2022.
//

#include "Minmax.h"
float piece_values[6] = {10, 30, 32, 50, 90, 0};

Minmax::Minmax(Board *g) {
    game = g;
}

float Minmax::minmax(int storage_position, int depth, int current_depth, float alpha, float beta) {
    if (current_depth == 0) {
        node_count++;
        if (node_count % 10000 == 0) {cout << node_count << " nodes rn" << endl;}
        return eval();
    } else if (current_depth == -1) {
        current_depth = depth;
    }
    // eventual move_sorting here
    vector<tuple<int, int, int>> moves;
    for (int i:(game->current_player?game->white_positions:game->black_positions)) {
        for (int dest_i = 0; dest_i < game->board[i].get_amount_moves(); dest_i++) {
            int dest = game->board[i].get_move(dest_i);
            if (game->board[i].get_type() == pawn && (dest/8 == 0 || dest/8 == 7)) {
                // promotions
                for (int prom = 2; prom < 6; prom++) {
                    moves.emplace_back(i, dest, prom);
                }
            } else {
                // normal moves
                moves.emplace_back(i, dest, 0);
            }
        }
    }
    // end of eventuality

    float max_score = -10000000;
    float min_score = 10000000;
    float eval;


    // simulate all moves of current depth
    for (const auto& current_move:moves) {
        // do move
        game->make_move(current_move);

        // update moves
        switch (game->generate_piece_moves()) {
            case not_over:
                break;
            case white_win:
                game->undo();
                if (current_depth == depth &&
                (best_moves_per_iteration[depth].empty() ||
                !game->current_player && (100000 - (float) (depth-current_depth + 1)) < min_score ||
                game->current_player && (100000 - (float) (depth-current_depth + 1)) > max_score)) {
                    best_moves_per_iteration[storage_position] = {current_move};
                } else if (current_depth == depth &&
                (!game->current_player && (100000 - (float) (depth - current_depth + 1)) == min_score ||
                game->current_player && (100000 - (float) (depth - current_depth + 1)) == max_score)) {
                    best_moves_per_iteration[storage_position].push_back(current_move);
                }
                return (100000) - (float) (depth - current_depth + 1);

            case black_win:
                game->undo();
                if (current_depth == depth &&
                (best_moves_per_iteration[depth].empty() ||
                !game->current_player && (-100000 + (float) (depth-current_depth + 1)) < min_score ||
                 game->current_player && (-100000 + (float) (depth-current_depth + 1)) > max_score)) {
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
                (best_moves_per_iteration[depth].empty() ||
                !game->current_player && 0 < min_score ||
                game->current_player && 0 > max_score)) {
                    best_moves_per_iteration[storage_position] = {current_move};
                } else if (current_depth == depth &&
                (!game->current_player && 0 == min_score ||
                game->current_player && 0 == max_score)) {
                    best_moves_per_iteration[storage_position].push_back(current_move);
                }
                return 0;
        }
        eval = minmax(storage_position, depth, current_depth-1, alpha, beta);
        game->undo();

        // evaluate the results depending on color
        if (game->current_player) {
            // adding best moves stuff
            if (eval > max_score) {
                max_score = eval;
                if (current_depth == depth) {best_moves_per_iteration[storage_position] = {current_move};}
            } else if (eval == max_score && current_depth == depth) {
                best_moves_per_iteration[storage_position].push_back(current_move);
            }

            // pruning stuff
            alpha = max(alpha, eval);


        } else {
            // adding best moves stuff
            if (eval < min_score) {
                min_score = eval;
                if (current_depth == depth) {
                    best_moves_per_iteration[storage_position] = {current_move};
                }
            } else if (eval == min_score && current_depth == depth) {
                best_moves_per_iteration[storage_position].push_back(current_move);
            }

            // pruning stuff
            beta = min(beta, eval);
        }
        if (beta <= alpha) {break;}
    }

    return (game->current_player?max_score: min_score);
}

float Minmax::eval() {
    float eval = 0;
    for (auto&i:game->white_positions) {
        eval += piece_values[game->board[i].get_type()-1];
    }
    for (auto&i:game->black_positions) {
        eval -= piece_values[game->board[i].get_type()-1];
    }
    return eval;
}

tuple<int, int, int> Minmax::get_move(bool use_depth, int depth_or_time) {
    if (use_depth) {
        best_moves_per_iteration.clear();
        node_count = 0;
        best_moves_per_iteration.emplace_back();
        //game->generate_piece_moves();
        minmax(0, depth_or_time);
        for (auto&bestmove:best_moves_per_iteration[0]) {
            cout << get<0>(bestmove) << " " << get<1>(bestmove) << endl;
        }
        cout << node_count << " nodes this move" << endl;
        return best_moves_per_iteration[0].at(rand()%best_moves_per_iteration[0].size());
    }
    else {
        return {};
    }
    ;
}
