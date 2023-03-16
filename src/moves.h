#pragma once

#include "board.h"

typedef Position Move[2];

// get legal moves for a piece on a square
// returns the number of allowed moves
// if moves is not NULL, the legal moves will be stored in *moves.
size_t getLegalMoves(Board *b, Position p, Position *moves);
Colour isCheckM(Board *b, Colour colour, Move m);
Colour isCheck(Board *b, Colour colour);

// Try to move a piece, it will return true for success.
// you have them somewhere.
bool move(Board *b, Move m);
