#include "Board.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Board b = Board("start");
    b.generate_piece_moves();
    return 0;
}
