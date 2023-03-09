#pragma once

#include "board.hpp"

namespace Moves
{

typedef Board::Position Move[2];

// get legal moves for a piece on a square
// returns the number of allowed moves
// if moves is not NULL, the legal moves will be stored in *moves.
size_t
getLegalMoves(Board::Board *b, Board::Position p, Board::Position *moves);
Pieces::Colour isCheck(Board::Board *b, Pieces::Colour colour, Move m);
Pieces::Colour isCheck(Board::Board *b, Pieces::Colour colour);

// Try to move a piece, it will return true for success.
// you have them somewhere.
bool move(Board::Board *b, Move m, Board::Position *moves, size_t moveCount);

} // namespace Moves