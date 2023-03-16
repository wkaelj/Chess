#pragma once

// Render a chessboard

#include "../board.h"
#include "../moves.h"

typedef struct BoardRender BoardRender;

BoardRender *create_board_render(
    const char *boardTexture,
    const char *pieceTexture,
    const char *hoverTexture,
    const char *legalMoveTexture,
    Colour colour);
void destroy_board_render(BoardRender *r);
void draw(Board *board);
void setPlayerColour(Colour c);
Colour getPlayerColour();

// check if the game should attempt to quit
bool board_render_quit(const BoardRender *render);

void board_render_update(BoardRender *render);

void board_render_draw(BoardRender *render, Board *b);