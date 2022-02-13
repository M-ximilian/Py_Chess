#include "Board.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("8/RpPk3R/8/3rN3/1N6/3Q4/8/1K6 b - - 0 1");
    b.generate_piece_moves();
    return 0;
}
