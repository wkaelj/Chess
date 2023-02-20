#include "moves.hpp"

#include <cassert>
#include <math.h>

namespace Moves
{

bool withinBoaed(int8_t position);
size_t getLegalColumn(
    Board::Board *b,
    Board::Position p,
    Board::Position *moves,
    size_t *moveIndex);
size_t getLegalRow(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex);
size_t getLegalDiagonals(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex);

size_t getLegalMoves(Board::Board *b, Board::Position p, Board::Position *moves)
{
    size_t movesIndex = 0;
    getLegalColumn(b, p, moves, &movesIndex);
    getLegalRow(b, p, moves, &movesIndex);
    getLegalDiagonals(b, p, moves, &movesIndex);
    return movesIndex;
}

bool move(Board::Board *b, Move m, Board::Position *moves, size_t moveCount)
{

    for (size_t i = 0; i < moveCount; i++)
    {
        if (moves[i] == m[1])
        {
            // sounds
            if ((Board::getPiece(b, m[1]) & 0x7f) != Pieces::BLANK)
            {
                if (b->sounds.capture)
                    b->sounds.capture->play();
            }
            else
            {
                if (b->sounds.move)
                    b->sounds.move->play();
            }
            Board::movePiece(b, m[0], m[1]);
            b->lastMove[0] = m[0];
            b->lastMove[1] = m[1];
            return true;
        }
    }
    return false;
}

//
// helpers
//

// assert a position is within the board
bool withinBoard(int8_t position) { return position >= 0 && position < 64; }

// if true stop looping
bool checkIntercept(
    Board::Board *b,
    Board::Position p,
    Pieces::Colour c,
    Board::Position *moves,
    size_t *moveIndex)
{
    Piece piece = Board::getPiece(b, p);
    if (piece == Pieces::BLANK)
    {
        if (moves)
            moves[*moveIndex] = p;
        *moveIndex += 1;
        return false;
    }
    else
    {
        // if a capture is legal, add it then exit the loop
        if (Pieces::getColour(p) != c)
        {
            if (moves)
                moves[*moveIndex] = p;
            *moveIndex += 1;
        }
        return true;
    }
}

size_t getLegalColumn(
    Board::Board *b,
    Board::Position p,
    Board::Position *moves,
    size_t *moveIndex)
{
    assert(b && moveIndex);
    assert(*moveIndex < 64);
    Pieces::Colour pieceColour = Pieces::getColour(Board::getPiece(b, p));

    // count moves down
    for (uint8_t i = p + 8; withinBoard(i); i += 8)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }

    // count moves up
    for (int8_t i = p - 8; withinBoard(i); i -= 8)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }
    return *moveIndex; // return number of legal moves
}

size_t getLegalRow(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex)
{
    assert(b && moveIndex);
    assert(*moveIndex < 64);

    Pieces::Colour pieceColour =
        Pieces::getColour(Board::getPiece(b, position));

    // count moves right
    for (int8_t i = position + 1;
         position % 8 != 7 && i % 8 != 0 && withinBoard(i);
         i++)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }

    // count moves left
    for (int8_t i = position - 1;
         (position % 8 != 0) && (i % 8 != 7) && withinBoard(i);
         i--)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }
    return *moveIndex; // return number of legal moves
}

size_t getLegalDiagonals(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex)
{
    Pieces::Colour pieceColour =
        Pieces::getColour(Board::getPiece(b, position)); // down and right

    // down right
    for (int8_t i = position + 9; i % 8 != 0 && withinBoard(i); i += 9)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }
    // up left
    for (int8_t i = position - 9; i % 8 != 7 && withinBoard(i); i -= 9)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }
    // down left
    for (int8_t i = position + 7; i % 8 != 7 && withinBoard(i); i += 7)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }
    // up right
    for (int8_t i = position - 7; i % 8 != 0 && withinBoard(i); i -= 7)
    {
        if (checkIntercept(b, i, pieceColour, moves, moveIndex))
            break;
    }
    return 0;
}

} // namespace Moves
