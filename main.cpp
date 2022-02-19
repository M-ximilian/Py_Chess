#include "Board.h"
#include <limits>

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("1q2k3/8/7p/7P/2Q5/3R4/1q6/K7 w - - 0 1");
    auto time_before = chrono::high_resolution_clock::now();
    game_ends g;
    for (int i = 0; i < 1; i++) {
        g = b.generate_piece_moves();
        //cout << g << endl;
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    cout << "Time needed: " << time / std::chrono::nanoseconds(1) << endl;
    return 0;
}
