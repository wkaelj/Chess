#include <assert.h>
#include <stdio.h>

#include "board.h"

#include "render/render.h"

int main(
    __attribute_maybe_unused__ int argc, __attribute_maybe_unused__ char **argv)
{
    Board b = createBoard();

    loadPosition(
        &b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    BoardRender *render = create_board_render(
        "/home/kael/Code/Chess/textures/board.png",
        "/home/kael/Code/Chess/textures/pieces.png",
        "/home/kael/Code/Chess/textures/hover.png",
        "/home/kael/Code/Chess/textures/legal_move.png",
        COLOUR_WHITE);

    while (board_render_quit(render) == false)
    {
        board_render_update(render);
        board_render_draw(render, &b);
    }

    printf("Loop over\n");

    destroy_board_render(render);

    return 0;
}