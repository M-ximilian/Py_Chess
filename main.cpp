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
    Board b = Board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    //b.generate_piece_moves();
    //b.make_move(26,34, 0);
    //b.generate_piece_moves();
    /*b.make_move(41, 48, 0);
    b.generate_piece_moves();
    b.make_move(0, 1, 0);
    b.generate_piece_moves();
    b.make_move(51, 35, 0);
    b.generate_piece_moves();*/


    map<string, int> storage;
    cout << b.perft(4, &storage) << endl;
    string real_counts = "b7b6: 35\n"
                         "c7c6: 35\n"
                         "d7d6: 34\n"
                         "d7d5: 35\n"
                         "g7h6: 30\n"
                         "a5b3: 34\n"
                         "a5c4: 34\n"
                         "a5c6: 35\n"
                         "f6e4: 32\n"
                         "f6g4: 32\n"
                         "f6d5: 35\n"
                         "f6h5: 34\n"
                         "f6g8: 34\n"
                         "g6e4: 33\n"
                         "g6f5: 35\n"
                         "g6h5: 34\n"
                         "a7c5: 5\n"
                         "a7b6: 34\n"
                         "a7b8: 35\n"
                         "a8b8: 34\n"
                         "a8c8: 34\n"
                         "a8d8: 34\n"
                         "h8f8: 34\n"
                         "h8g8: 34\n"
                         "a3a2: 34\n"
                         "a3b3: 35\n"
                         "a3c3: 36\n"
                         "a3d3: 33\n"
                         "a3e3: 3\n"
                         "a3f3: 32\n"
                         "a3a4: 34\n"
                         "a3b4: 32\n"
                         "e8e7: 34\n"
                         "e8d8: 34\n"
                         "e8f8: 34\n"
                         "e8c8: 34";
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
        cout << wrong_indices[i] << " " << wrong_counters[i] << endl;
    }
    return 0;
}