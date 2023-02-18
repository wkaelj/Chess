#pragma once

#include "board.hpp"

namespace Moves
{

typedef Board::Position Move[2];

bool isLegal(Board::Board *b, Move m);

// manage moves, like castling and en passant
// checks that move is legal
bool move(Board::Board *b, Move m);

} // namespace Moves