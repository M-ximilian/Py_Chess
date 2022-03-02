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


// no check blocking via double pawn move
// no check blocking via taken attacker with pawn
// pawns only block 1 square in front of them
int main() {
    // perft 2 fen: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1, works
    // perft 3 fen: 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1
    Board b = Board("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    b.generate_piece_moves();
    b.make_move(12, 28, 0);
    b.draw();
    b.generate_piece_moves();
    /*b.make_move(60, 59, 0);
    b.draw();
    b.generate_piece_moves();
    b.make_move(36, 42, 0);
    b.generate_piece_moves();
    b.draw();
    b.generate_piece_moves();*/

    map<string, int> storage;
    cout << b.perft(1, &storage) << endl;
    string real_counts = "e2e3: 15\n"
                         "g2g3: 4\n"
                         "a5a6: 15\n"
                         "e2e4: 16\n"
                         "g2g4: 17\n"
                         "b4b1: 16\n"
                         "b4b2: 16\n"
                         "b4b3: 15\n"
                         "b4a4: 15\n"
                         "b4c4: 15\n"
                         "b4d4: 15\n"
                         "b4e4: 15\n"
                         "b4f4: 2\n"
                         "a5a4: 15";
    string index, counter;
    bool at_index = true, at_counter = false;
    vector<string> wrong_indices;
    vector<int> wrong_counters;
    for (char real_count : real_counts) {
        if (real_count == '\n') {
            if (storage[index] != stoi(counter)) {
                wrong_indices.push_back(index);
                wrong_counters.push_back(storage[index]- stoi(counter));
            }
            index.clear(); counter.clear();at_index = true; at_counter = false; continue;
        }
        if (real_count == ' ') {continue;}
        if (real_count == ':') {at_counter = true; at_index = false; continue;}
        if (at_index) {index.push_back(real_count);}
        else if (at_counter) {counter.push_back(real_count);}

    }
    for (int i = 0; i < wrong_indices.size(); i++) {
        //cout << wrong_indices[i] << " " << wrong_counters[i] << endl;
    }
    return 0;
}