#include "moves.h"
#include "board.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

bool withinBoard(int8_t position);
void getLegalColumn(Board *b, Position p, Position *moves, size_t *moveIndex);
void getLegalRow(
    Board *b, Position position, Position *moves, size_t *moveIndex);
void getLegalDiagonals(
    Board *b, Position position, Position *moves, size_t *moveIndex);
void getLegalKnightMoves(
    Board *b, Position position, Position *moves, size_t *moveIndex);
void getLegalKingMoves(
    Board *b, Position position, Position *moves, size_t *moveIndex);
void getLegalPawnMoves(
    Board *b, Position position, Position *moves, size_t *moveIndex);

Colour isCheckInternal(
    Board *b,
    Colour colour,
    Move m,
    Position *legalMoves,
    size_t legalMoveCount);

size_t getLegalMoves(Board *b, Position p, Position *moves)
{
    size_t movesIndex = 0;

    Piece piece = getPiece(b, p);
    switch (piece & 0x7f)
    {
    case PIECE_PAWN:
        getLegalPawnMoves(b, p, moves, &movesIndex);
        // handle en passant
        break;
    case PIECE_BISHOP: getLegalDiagonals(b, p, moves, &movesIndex); break;
    case PIECE_ROOK:
        getLegalColumn(b, p, moves, &movesIndex);
        getLegalRow(b, p, moves, &movesIndex);
        break;
    case PIECE_KNIGHT: getLegalKnightMoves(b, p, moves, &movesIndex); break;
    case PIECE_KING: getLegalKingMoves(b, p, moves, &movesIndex); break;
    default:
        getLegalColumn(b, p, moves, &movesIndex);
        getLegalRow(b, p, moves, &movesIndex);
        getLegalDiagonals(b, p, moves, &movesIndex);
        break;
    }

    Colour colour = getColour(getPiece(b, p));
    assert(colour != COLOUR_NONE);
    for (size_t i = 0; i < movesIndex; i++)
    {
        Move m = {p, moves[i]};
        if (isCheckInternal(b, colour, m, moves, movesIndex) == colour)
            moves[i] = UINT8_MAX;
    }
    return movesIndex;
}

bool move(Board *b, Move m)
{
    assert(m[1] < 64 && m[0] < 64);
    Position moves[32];
    size_t moveCount = getLegalMoves(b, m[0], moves);

    // ensure the move is legal
    bool found = false;
    for (size_t i = 0; i < moveCount; i++)
    {
        if (moves[i] < 64 && moves[i] == m[1])
        {
            found = true;
            break;
        }
    }
    if (found == false)
        return false;

    // move the piece
    movePiece(b, m[0], m[1]);
    b->lastMove[0] = m[0];
    b->lastMove[1] = m[1];
    b->moveCount++;
    return true;
}

//
// helpers
//

// is check overload if no move needed
Colour isCheck(Board *b, Colour colour)
{
    Move m = {255, 255};
    return isCheckM(b, colour, m);
}

// check if the king of colour c is in check. If so, it will return the
// colour of the piece being checked. If the other king is checked, it will
// return that king instead.
Colour isCheckM(Board *b, Colour colour, Move m)
{
    Position legalMoves[32];
    size_t legalMoveCount;
    legalMoveCount = getLegalMoves(b, m[0], legalMoves);
    return isCheckInternal(b, colour, m, legalMoves, legalMoveCount);
}

Colour isCheckInternal(
    Board *b,
    Colour colour,
    Move m,
    Position *legalMoves,
    size_t legalMoveCount)
{

    // make the move being tested, and store it so it can be moved back
    // this will probably cause a ton of problems in the future
    Piece capturedPiece = PIECE_BLANK;
    if (m[0] < 64 && m[1] < 64)
    {
        capturedPiece = getPiece(b, m[1]);
        movePiece(b, m[0], m[1]);
    }

    Colour checkedColour = COLOUR_NONE;
    // iterate through the entire board to see if any pieces have check
    for (Position position = 0; position < 64 && checkedColour != colour;
         position++)
    {
        Piece movedPiece = getPiece(b, position);
        // if the moved piece is not blank get the legal moves and see if any of
        // them check
        if ((movedPiece & 0x7f) != PIECE_BLANK)
        {
            for (size_t move = 0; move < legalMoveCount; move++)
            {
                // if a legal move attacks a king it is check
                // do not need to check colour, as moving onto the own colour
                // king is not allowed by the rules
                Piece attackedPiece = getPiece(b, legalMoves[move]);
                if ((attackedPiece & 0x7f) == PIECE_KING)
                {
                    Colour c = getColour(attackedPiece);
                    if (c == colour)
                        checkedColour = c;
                    else if (checkedColour == COLOUR_NONE)
                        checkedColour = c;
                }
            }
        }
    }

    // move piece back
    if (m[0] < 64 && m[1] < 64)
    {
        movePiece(b, m[1], m[0]);
        setPiece(b, m[1], capturedPiece);
    }

    return checkedColour;
}
// assert a position is within the board
bool withinBoard(int8_t position) { return position >= 0 && position < 64; }

// if true stop looping
// check if a piece has hit another piece while moving
bool checkIntercept(
    Board *b, Position p, Colour c, Position *moves, size_t *moveIndex)
{
    Piece piece = getPiece(b, p);
    if (piece == PIECE_BLANK)
    {
        if (moves)
            moves[*moveIndex] = p;
        *moveIndex += 1;
        return false;
    }
    else
    {
        // if a capture is legal, add it then exit the loop
        if (getColour(piece) != c)
        {
            if (moves)
                moves[*moveIndex] = p;
            *moveIndex += 1;
        }
        return true;
    }
}

void getLegalColumn(Board *b, Position p, Position *moves, size_t *moveIndex)
{
    assert(b && moveIndex);
    assert(*moveIndex < 64);
    Colour pieceColour = getColour(getPiece(b, p));

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
    Board *b, Position position, Position *moves, size_t *moveIndex)
{
    assert(b && moveIndex);
    assert(*moveIndex < 64);
    Colour pieceColour = getColour(getPiece(b, position));

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
    Board *b, Position position, Position *moves, size_t *moveIndex)
{
    Colour pieceColour = getColour(getPiece(b, position)); // down and right

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

// check if a square can be moved to, and if so add it to the move list
void handleMoveSquare(
    Board *b,
    Position position,
    Position *moves,
    size_t *moveIndex,
    Colour pieceColour,
    uint8_t move)
{
    // ensure move is within board, and does not wrap corner
    if (!withinBoard(position + move))
        return;

    Piece p = getPiece(b, position + move);
    if (p == PIECE_BLANK || getColour(p) != pieceColour)
    {
        moves[*moveIndex] = position + move;
        *moveIndex += 1;
    }
}

void getLegalKnightMoves(
    Board *b, Position position, Position *moves, size_t *moveIndex)
{

    Colour pieceColour = getColour(getPiece(b, position));

    // left 2
    if (position % 8 != 0 && position % 8 != 1)
    {
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, -10);
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, 6);
    }
    if (position % 8 != 0)
    {
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, -17);
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, 15);
    }
    // right
    if (position % 8 != 7 && position % 8 != 6)
    {
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, 10);
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, -6);
    }
    if (position % 8 != 7)
    {
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, -15);
        handleMoveSquare(b, position, moves, moveIndex, pieceColour, 17);
    }
}

void getLegalKingMoves(
    Board *b, Position position, Position *moves, size_t *moveIndex)
{
    const Piece p = getPiece(b, position);
    assert((p & 0x7f) == PIECE_KING);

    Colour colour = getColour(p);
    handleMoveSquare(b, position, moves, moveIndex, colour, -8);
    handleMoveSquare(b, position, moves, moveIndex, colour, 8);
    // left
    if (position % 8)
    {
        handleMoveSquare(b, position, moves, moveIndex, colour, -1);
        handleMoveSquare(b, position, moves, moveIndex, colour, -9);
        handleMoveSquare(b, position, moves, moveIndex, colour, 7);
    }
    // right
    if (position % 8 != 7)
    {
        handleMoveSquare(b, position, moves, moveIndex, colour, 1);
        handleMoveSquare(b, position, moves, moveIndex, colour, 9);
        handleMoveSquare(b, position, moves, moveIndex, colour, -7);
    }

    // castling
    if (colour == COLOUR_WHITE && b->w_castle_k)
        ;
}

void getLegalPawnMoves(
    Board *b, Position position, Position *moves, size_t *moveIndex)
{
    Piece p = getPiece(b, position);
    assert((p & 0x7f) == PIECE_PAWN);

    Colour c = getColour(p);
    assert(c != COLOUR_NONE);
    int direction;
    if (c == COLOUR_WHITE)
        direction = -1;
    if (c == COLOUR_BLACK)
        direction = 1;

    // if there is no piece in front of the pawn, it is a legal move
    const int forward = 8 * direction;
    if (withinBoard(position + forward) &&
        getPiece(b, position + forward) == PIECE_BLANK)
        handleMoveSquare(b, position, moves, moveIndex, c, forward);
    // if there is a piece to one side of the pawn, consider the move.
    const int side1 = 9 * direction;
    if (withinBoard(position + side1) &&
        getPiece(b, position + side1) != PIECE_BLANK)
        handleMoveSquare(b, position, moves, moveIndex, c, side1);
    // if there is a piece to the other side, consider it a move
    const int side2 = 7 * direction;
    if (withinBoard(position + side2) &&
        getPiece(b, position + side2) != PIECE_BLANK)
        handleMoveSquare(b, position, moves, moveIndex, c, side2);
}
