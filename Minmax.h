//
// Created by maxim on 07.03.2022.
//

#ifndef PY_CHESS_MINMAX_H
#define PY_CHESS_MINMAX_H
#include "Board.h"

class Minmax {
public:
    explicit Minmax(Board *g);

    tuple<int, int, int> get_move(bool use_depth, int depth_or_time = 0);

private:
    Board * game;
    vector<vector<tuple<int, int, int>>> best_moves_per_iteration;
    bool timeout = false;
    int node_count;

    float minmax(int storage_position, int depth, int current_depth = -1, float alpha = -numeric_limits<float>::infinity(), float beta = numeric_limits<float>::infinity());

    void order_moves();

    void sort_moves();

    void time_search(int max_time);

    float evaluate();
};


#endif //PY_CHESS_MINMAX_H
