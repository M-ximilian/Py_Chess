#include "Board.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("1r5k/8/8/8/8/b7/1K6/8 w - - 0 1");
    b.generate_piece_moves();
    return 0;
}
