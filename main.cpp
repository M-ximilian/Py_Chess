#include "Minmax.h"







int main() {
    int used_seed = (int) time(nullptr);
    cout << used_seed << endl;
    srand(1646816751);

    for (int i = 0; i < 10000000; i++) {
        //n2nn2n/8/8/n3n2n/n6n/8/8/n2nn2n w - - 0 1
        Board b = Board("start"); //break i == 5
        Minmax m = Minmax(&b);
        while (true) {
            b.draw();
            if (b.generate_piece_moves() != not_over) {
                cout << "over" << endl;
                break;
            }
            string usermove;
            cin >> usermove;
            cout << "move " << (tolower(usermove[0]) - 97) + 8 * (usermove[1] - 49) << " to "
                 << (tolower(usermove[2]) - 97) + 8 * (usermove[3] - 49) << " pro "
                 << (usermove.size() == 5 ? convert_piece(tolower(usermove[4])) : 0) << endl;
            b.make_move((tolower(usermove[0]) - 97) + 8 * (usermove[1] - 49),
                      (tolower(usermove[2]) - 97) + 8 * (usermove[3] - 49),
                      usermove.size() == 5 ? convert_piece(tolower(usermove[4])) : 0);

            b.draw();
            if (b.generate_piece_moves() != not_over) {
                cout << "also over" << endl;
                break;
            }
            auto time_before = chrono::high_resolution_clock::now();
            b.make_move(m.get_move(true, 5));
            auto time_after = chrono::high_resolution_clock::now();
            auto delta_time = time_after-time_before;
            cout << "Total Time: " << delta_time/chrono::milliseconds(1) << endl;
        }



        }
    return 0;
}



/*
int main() {
    // perft 2 fen: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1, works
    // perft 3 fen: 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1
    Board b = Board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");



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
*/