#include "board.hpp"

#include <assert.h>

namespace Board
{

// board index 0 is bottom left, index 64 is top right,
// goes horizontal

Position generatePosition(uint8_t x, uint8_t y)
{
    assert(x < 8);
    assert(y < 8);
    return x * 8 + y;
}

Piece getPiece(Board *b, Position p)
{
    // find the number of tiles below the row
    assert(p < 64);

    return b->tiles[p];
}

void setPiece(Board *b, Position p, Piece piece)
{
    assert(p < 64);
    assert(piece < Pieces::PIECE_MAX);

    b->tiles[p] = piece;
}

void movePiece(Board *b, Position initial, Position final)
{
    setPiece(b, final, getPiece(b, initial)); // set final square to initial
    setPiece(b, initial, Pieces::BLANK);      // set initial square to blank
}

int generateChecksum(Board *b) { return 0; }

} // namespace Board