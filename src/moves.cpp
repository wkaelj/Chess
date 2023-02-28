#include "moves.hpp"

#include <cassert>
#include <math.h>

namespace Moves
{

bool withinBoard(int8_t position);
void getLegalColumn(
    Board::Board *b,
    Board::Position p,
    Board::Position *moves,
    size_t *moveIndex);
void getLegalRow(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex);
void getLegalDiagonals(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex);
void getLegalKnightMoves(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex);

Pieces::Colour isCheck(Board::Board *b, Pieces::Colour colour, Move m);
Pieces::Colour isCheck(Board::Board *b, Pieces::Colour colour);
size_t getLegalMoves(Board::Board *b, Board::Position p, Board::Position *moves)
{
    size_t movesIndex = 0;

    Piece piece = Board::getPiece(b, p);
    switch (piece & 0x7f)
    {
    case Pieces::ROOK:
        getLegalColumn(b, p, moves, &movesIndex);
        getLegalRow(b, p, moves, &movesIndex);
        break;
    case Pieces::KNIGHT: getLegalKnightMoves(b, p, moves, &movesIndex); break;
    default:
        getLegalColumn(b, p, moves, &movesIndex);
        getLegalRow(b, p, moves, &movesIndex);
        getLegalDiagonals(b, p, moves, &movesIndex);
        break;
    }
    return movesIndex;
}

bool move(Board::Board *b, Move m, Board::Position *moves, size_t moveCount)
{

    for (size_t i = 0; i < moveCount; i++)
    {
        if (moves[i] == m[1])
        {
            Pieces::Colour movedColour =
                Pieces::getColour(Board::getPiece(b, m[0]));
            if (isCheck(b, movedColour, m) == movedColour)
                return false;
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
            if (isCheck(b, Pieces::Colour::BLACK) != Pieces::NONE)
            {
                printf("Check\n");
            }
            return true;
        }
    }
    return false;
}

//
// helpers
//

// is check overload if no move needed
Pieces::Colour isCheck(Board::Board *b, Pieces::Colour colour)
{
    Move m = {255, 255};
    return isCheck(b, colour, m);
}

// check if the king of colour c is in check. If so, it will return the
// colour of the piece being checked. If the other king is checked, it will
// return that king instead.
Pieces::Colour isCheck(Board::Board *b, Pieces::Colour colour, Move m)
{
    // make the move being tested, and store it so it can be moved back
    // this will probably cause a ton of problems in the future
    Piece capturedPiece;
    if (m[0] < 64 && m[1] < 64)
    {
        capturedPiece = Board::getPiece(b, m[1]);
        Board::movePiece(b, m[0], m[1]);
    }

    Pieces::Colour checkedColour = Pieces::Colour::NONE;
    // iterate through the entire board to see if any pieces have check
    for (Board::Position position = 0; position < 64 && checkedColour != colour;
         position++)
    {
        Piece movedPiece = Board::getPiece(b, position);
        // if the moved piece is not blank get the legal moves and see if any of
        // them check
        if ((movedPiece & 0x7f) != Pieces::BLANK)
        {
            Board::Position legalMoves[64];
            size_t legalMoveCount = getLegalMoves(b, position, legalMoves);
            for (size_t move = 0; move < legalMoveCount; move++)
            {
                // if a legal move attacks a king it is check
                // do not need to check colour, as moving onto the own colour
                // king is not allowed by the rules
                Piece attackedPiece = Board::getPiece(b, legalMoves[move]);
                if ((attackedPiece & 0x7f) == Pieces::KING ||
                    (attackedPiece & 0x7f) == Pieces::KING_CASTLE)
                {
                    Pieces::Colour c = Pieces::getColour(attackedPiece);
                    if (c == colour)
                        checkedColour = c;
                    else if (checkedColour == Pieces::Colour::NONE)
                        checkedColour = c;
                }
            }
        }
    }

    // move piece back
    if (m[0] < 64 && m[1] < 64)
    {
        Board::movePiece(b, m[1], m[0]);
        Board::setPiece(b, m[1], capturedPiece);
    }

    return checkedColour;
}

// assert a position is within the board
bool withinBoard(int8_t position) { return position >= 0 && position < 64; }

// if true stop looping
// check if a piece has hit another piece while moving
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
        if (Pieces::getColour(piece) != c)
        {
            if (moves)
                moves[*moveIndex] = p;
            *moveIndex += 1;
        }
        return true;
    }
}

void getLegalColumn(
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
}

void getLegalRow(
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
}

void getLegalDiagonals(
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
}

void handleKnightMove(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex,
    Pieces::Colour pieceColour,
    uint8_t move)
{
    if (withinBoard(position + move) &&
        Pieces::getColour(Board::getPiece(b, position + move)) != pieceColour)
    {
        moves[*moveIndex] = position + move;
        *moveIndex += 1;
    }
}

void getLegalKnightMoves(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex)
{

    Pieces::Colour pieceColour =
        Pieces::getColour(Board::getPiece(b, position));

    handleKnightMove(b, position, moves, moveIndex, pieceColour, 17);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, -17);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, 15);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, -15);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, 10);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, -10);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, 6);
    handleKnightMove(b, position, moves, moveIndex, pieceColour, -6);
}

void getLegalKingMoves(
    Board::Board *b,
    Board::Position position,
    Board::Position *moves,
    size_t *moveIndex)
{
}

} // namespace Moves
