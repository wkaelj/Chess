#pragma once

#include <stdint.h>

#include "render/audio.hpp"

typedef int8_t Piece;

namespace Pieces
{

// negative peices are black
enum
{
    BLANK = 0,
    PAWN  = 1,
    PAWN_EN_PASSANT, // can be captured en passant
    KNIGHT,
    BISHOP,
    ROOK,
    ROOK_CASTLE,
    QUEEN,
    KING,
    KING_CASTLE,
    PIECE_MAX,
};

enum Colour
{
    BLACK = 0,
    WHITE = 1,
    NONE,
};

// get the colour of a piece
inline Pieces::Colour getColour(Piece p) { return (p & 0x80) ? WHITE : BLACK; }

// set the colour of a piece
inline void setColour(Piece *p, Colour c)
{
    switch (c)
    {
    case Colour::BLACK: *p &= 0x7f; break;
    case Colour::WHITE: *p |= 0x80; break;
    default: break;
    }
}

} // namespace Pieces

namespace Board
{

// represents a pieces position
typedef uint8_t Position;

/*
Board goes from left to right and up
16 --->
8  --->
0  --->
*/
struct Board
{
    uint8_t tiles[64];
    Position lastMove[2];

    struct
    {
        Audio::Sound *move;
        Audio::Sound *capture;
        Audio::Sound *check;
    } sounds;
};

Board createBoard(
    Audio::Sound *move, Audio::Sound *capture, Audio::Sound *check);

// create a checksum of the board in order to verify moves online
int generateChecksum(Board *b);

Piece getPiece(Board *b, uint8_t position);

void movePiece(Board *b, Position initial, Position final);

void setPiece(Board *b, Position p, Piece piece);

// generate a position using the rank and file
Position generatePosition(uint8_t file, uint8_t rank);

// load a position from a string to a board
void loadPosition(Board *b, const char *pos);

} // namespace Board