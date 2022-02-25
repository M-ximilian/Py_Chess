#include "Board.h"

int main() {
    int used_seed = (int) time(nullptr);
    srand(1645809522);
    std::cout << "Hello, World!" << std::endl;
    auto time_before = chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        Board b = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1"); //break i == 5
        b.random_games();
        cout << "done" << endl;
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    cout << "Time needed: " << time / std::chrono::nanoseconds(1) << endl;
    return 0;
}
