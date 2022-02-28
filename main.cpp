#include "Board.h"

int main() {
    int used_seed = (int) time(nullptr);
    srand(used_seed);
    std::cout << "Hello, World!" << std::endl;
    auto time_before = chrono::high_resolution_clock::now();
    auto before = time(nullptr);
    auto after = time(nullptr);

    for (int i = 0; i < 10000000; i++) {
        //n2nn2n/8/8/n3n2n/n6n/8/8/n2nn2n w - - 0 1
        Board b = Board("start"); //break i == 5
        b.random_games(i);
        //cout << "done" << endl;
        if (i % 5000 == 0) {
            after = time(nullptr);
            cout << i << " Games in " << after-before << " Sek, " << i/(max(after-before, (long long) 1)) << " Games per Sek" << endl;
        }
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    cout << "Time needed: " << time / std::chrono::milliseconds(1) << endl;
    return 0;
}
