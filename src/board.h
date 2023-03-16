#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t Piece;

// negative peices are black
enum
{
    PIECE_BLANK = 0,
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN,
    PIECE_KING,
    PIECE_PIECE_MAX,
};

typedef enum
{
    COLOUR_BLACK = 0,
    COLOUR_WHITE = 1,
    COLOUR_NONE,
} Colour;

// get the colour of a piece
inline Colour getColour(Piece p)
{
    return (p & 0x80) ? COLOUR_WHITE : COLOUR_BLACK;
}

// set the colour of a piece
inline void setColour(Piece *p, Colour c)
{
    switch (c)
    {
    case COLOUR_BLACK: *p &= 0x7f; break;
    case COLOUR_WHITE: *p |= 0x80; break;
    default: break;
    }
}

// represents a pieces position
typedef uint8_t Position;

/*
Board goes from left to right and up
16 --->
8  --->
0  --->
*/
typedef struct
{
    uint8_t tiles[64];
    Position lastMove[2];

    int en_passant;
    bool w_castle_k, w_castle_q;
    bool b_castle_k, b_castle_q;

    size_t moveCount;
} Board;

Board createBoard();

// create a checksum of the board in order to verify moves online
int generateChecksum(Board *b);

Piece getPiece(Board *b, uint8_t position);

void movePiece(Board *b, Position initial, Position final);

void setPiece(Board *b, Position p, Piece piece);

// generate a position using the rank and file
Position generatePosition(uint8_t file, uint8_t rank);

// load a position from a string to a board
void loadPosition(Board *b, const char *pos);