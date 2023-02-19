#include "moves.hpp"

#include <cassert>
#include <math.h>

namespace Moves
{

// assert a position is within the board
bool withinBoard(uint8_t position) { return position < 64; }

// check if a move up or down a column is legal
bool legalColumn(Board::Board *b, Move m)
{
    // ensure pieces are on the same column
    if (m[1] % 8 != m[0] % 8)
        return false;

    Piece p = Board::getPiece(b, m[0]);

    if (m[1] > m[0])
    {
        for (uint8_t i = m[0] + 8; i < m[1] && withinBoard(i); i += 8)
        {
            if (Board::getPiece(b, i) != Pieces::BLANK)
                return false;
        }
    }
    else
    {
        for (uint8_t i = m[0] - 8; i > m[1] && withinBoard(i); i -= 8)
        {
            if (Board::getPiece(b, i) != Pieces::BLANK)
                return false;
        }
    }

    // cannot capture if the peice is the same colour
    if (Pieces::getColour(Board::getPiece(b, m[1])) == Pieces::getColour(p))
        return false;

    return true;
}

bool legalRow(Board::Board *b, Move m)
{
    // find the number of squares on the right and left of initial square
    int8_t left = -(m[0] % 8), right = 8 - (m[0] % 8);

    assert((m[0] + left) % 8 == 0);
    assert((m[0] + right) % 8 == 0);

    int8_t diff = m[1] - m[0];

    // move must be within the right and left
    if (diff > right && diff < left)
        return false;

    if (diff > 0)
    {
        for (uint8_t i = m[0]; i < m[1]; i++)
            if (Board::getPiece(b, i) != Pieces::BLANK)
                return false;
    }
    else
    {
        for (uint8_t i = m[0]; i > m[1]; i--)
            if (Board::getPiece(b, i) != Pieces::BLANK)
                return false;
    }

    // cannot take square of same colour
    if (Pieces::getColour(Board::getPiece(b, m[1])) ==
        Pieces::getColour(Board::getPiece(b, m[0])))
        return false;

    return true;
}

bool legalDiagonal(Board::Board *b, Move m)
{
    // final position must be within board
    if (!withinBoard(m[1]))
        return false;

    int8_t diff = m[1] - m[0];

    if (diff % 7 != 0 && diff % 9 != 0)
        return false;

    return true;
}

bool isLegal(Board::Board *b, Move m)
{
    // cannot move to same square
    if (m[1] == m[0])
        return false;

    Piece movedPiece    = Board::getPiece(b, m[0]);
    Piece capturedPiece = Board::getPiece(b, m[1]);

    switch (movedPiece & 0x7f)
    {
    case Pieces::PAWN:
        if (1)
            break;
    }

    return false;
}

} // namespace Moves
