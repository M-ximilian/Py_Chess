//
// Created by maxim on 07.03.2022.
//

#ifndef PY_CHESS_MINMAX_H
#define PY_CHESS_MINMAX_H
#include "Board.h"

class Minmax {
public:
    explicit Minmax(Board *g);

    tuple<int, int, int> get_move(bool use_depth, int depth_or_time);

private:
    Board * game;
    vector<vector<tuple<int, int, int>>> best_moves_per_iteration;
    bool timeout = false;

    int minmax(int depth, int current_depth = -1, float alpha = -numeric_limits<float>::infinity(), int beta = numeric_limits<float>::infinity());

    void order_moves();

    void sort_moves();

    void time_search(int max_time);
};


#endif //PY_CHESS_MINMAX_H
