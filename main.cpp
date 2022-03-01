#include "Board.h"

/*int main() {
    int used_seed = (int) time(nullptr);
    srand(used_seed);
    std::cout << "Hello, World!" << std::endl;
    auto time_before = chrono::high_resolution_clock::now();
    long long time_spent = 0;

    for (int i = 0; i < 10000000; i++) {
        //n2nn2n/8/8/n3n2n/n6n/8/8/n2nn2n w - - 0 1
        Board b = Board("start"); //break i == 5
        auto before = time(nullptr);
        b.random_games(i);
        time_spent += time(nullptr)-before;

        //cout << "done" << endl;
        if (i % 5000 == 0) {
            cout << i << " Games in " << time_spent << " Sek, " << i/(max(time_spent, (long long) 1)) << " Games per Sek" << endl;
        }
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - time_before;
    cout << "Time needed: " << time / std::chrono::milliseconds(1) << endl;
    return 0;
}
*/
int main() {
    // perf 2 fen: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1, works till perft 3
    Board b = Board("start");
    //b.generate_piece_moves();
    //b.make_move(50, 34, 0);
    map<string, int> storage;
    cout << b.perft(5, &storage) << endl;
    string real_counts = "a2a3: 181046\n"
                         "b2b3: 215255\n"
                         "c2c3: 222861\n"
                         "d2d3: 328511\n"
                         "e2e3: 402988\n"
                         "f2f3: 178889\n"
                         "g2g3: 217210\n"
                         "h2h3: 181044\n"
                         "a2a4: 217832\n"
                         "b2b4: 216145\n"
                         "c2c4: 240082\n"
                         "d2d4: 361790\n"
                         "e2e4: 405385\n"
                         "f2f4: 198473\n"
                         "g2g4: 214048\n"
                         "h2h4: 218829\n"
                         "b1a3: 198572\n"
                         "b1c3: 234656\n"
                         "g1f3: 233491\n"
                         "g1h3: 198502";
    string index, counter;
    bool at_index = true, at_counter = false;
    vector<string> wrong_indices;
    vector<int> wrong_counters;
    for (char real_count : real_counts) {
        if (real_count == '\n') {
            if (storage[index] != stoi(counter)) {
                wrong_indices.push_back(index);
                wrong_counters.push_back(stoi(counter)- storage[index]);
            }
            index.clear(); counter.clear();at_index = true; at_counter = false; continue;
        }
        if (real_count == ' ') {continue;}
        if (real_count == ':') {at_counter = true; at_index = false; continue;}
        if (at_index) {index.push_back(real_count);}
        else if (at_counter) {counter.push_back(real_count);}

    }
    for (int i = 0; i < wrong_indices.size(); i++) {
        cout << wrong_indices[i] << " " << wrong_counters[i] << endl;
    }
    return 0;
}