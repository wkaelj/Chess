#include <stdio.h>

#include "board.hpp"

#include "render/render.hpp"

int main(int argc, char **argv)
{
    printf("Hello World\n");

    Board::Board b;

    b.lastMove[0] = 3;
    b.lastMove[1] = 4;

    Board::setPiece(&b, 17, Pieces::QUEEN & 0x7f);
    Board::setPiece(&b, 63, Pieces::KING & 0x7f);
    Board::setPiece(&b, 6, Pieces::KNIGHT & 0x7f);

    for (uint i = 0; i < 64; i++)
    {
        printf("[%d]", b.tiles[i]);
        if (i % 8 == 0)
            printf("\n");
    }
    printf("\n");

    Render render{
        "./textures/board.png",
        "./textures/pieces.png",
        "./textures/hover.png",
        Pieces::Colour::WHITE};

    while (render.quit() == false)
    {
        render.draw(&b);
        render.pollEvents();
    }

    printf("Loop over");

    return 0;
}