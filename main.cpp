#include "Board.h"
#include <limits>

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    auto time_before = chrono::high_resolution_clock::now();
    game_ends g;
    for (int i = 0; i < 1; i++) {
        b.run();
        //cout << g << endl;
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    cout << "Time needed: " << time / std::chrono::nanoseconds(1) << endl;
    return 0;
}
