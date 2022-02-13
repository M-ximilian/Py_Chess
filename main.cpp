#include "Board.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("8/RpPk3R/8/3r4/8/3Q4/8/8 b - - 0 1");
    b.generate_piece_moves();
    return 0;
}
