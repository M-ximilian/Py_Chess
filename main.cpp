#include "Board.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("7k/8/2r1B3/8/1Q6/5R2/2K5/8 w - - 0 1");
    auto time_before = chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        b.generate_piece_moves();
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    cout << "Time needed: " << time / std::chrono::milliseconds(1) << endl;
    return 0;
}
