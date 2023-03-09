#include "board.hpp"

#include <assert.h>

#include <string.h>

namespace Board
{

// board index 0 is bottom left, index 64 is top right,
// goes horizontal

Board createBoard(
    Audio::Sound *move, Audio::Sound *capture, Audio::Sound *check)
{
    Board b;
    for (uint8_t i = 0; i < 64; i++)
        b.tiles[i] = Pieces::BLANK;
    b.lastMove[0] = UINT8_MAX;
    b.lastMove[1] = UINT8_MAX;
    b.sounds      = {move, capture, check};

    return b;
}

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
    if ((getPiece(b, final) & 0x7f) != Pieces::BLANK)
    {
        if (b->sounds.capture)
            b->sounds.capture->play();
    }
    else
    {
        if (b->sounds.move)
            b->sounds.move->play();
    }
    setPiece(b, final, getPiece(b, initial)); // set final square to initial
    setPiece(b, initial, Pieces::BLANK);      // set initial square to blank
}

int generateChecksum(Board *b) { return 0; }

Piece getPieceFromChar(char c)
{
    switch (c)
    {
    case 'p': return Pieces::PAWN | 0x80;
    case 'n': return Pieces::KNIGHT | 0x80;
    case 'b': return Pieces::BISHOP | 0x80;
    case 'r': return Pieces::ROOK | 0x80;
    case 'q': return Pieces::QUEEN | 0x80;
    case 'k': return Pieces::KING_CASTLE | 0x80;
    case 'P': return Pieces::PAWN & 0x7f;
    case 'N': return Pieces::KNIGHT & 0x7f;
    case 'B': return Pieces::BISHOP & 0x7f;
    case 'R': return Pieces::ROOK & 0x7f;
    case 'Q': return Pieces::QUEEN & 0x7f;
    case 'K': return Pieces::KING_CASTLE & 0x7f;
    }
    printf("Invalid piece '%c'", c);
    return Pieces::BLANK;
}
void loadPosition(Board *b, const char *fen)
{
    //     size_t len  = strlen(fen);
    //     uint8_t pos = 0;

    //     size_t field = 0; // < 6
    //     for (size_t i = 0; i < len; i++)
    //     {
    //         switch (fen[i])
    //         {
    //         case ' ': field++; break;
    //         case '/': pos = pos + 8 / 8;
    //             case ''
    //         }
    //     }
    // Initialize tiles array to 0
    memset(b->tiles, 0, sizeof(b->tiles));

    int i = 0, j = 0;
    while (*fen != '\0')
    {
        if (*fen == '/')
        {
            i++;
            j = 0;
        }
        else if (isdigit(*fen))
        {
            j += (*fen - '0');
        }
        else if (*fen == ' ')
        {
            break;
        }
        else
        {
            int piece = getPieceFromChar(*fen);
            if (piece != -1)
            {
                b->tiles[i * 8 + j] = piece;
                j++;
            }
        }
        fen++;
    }
}

} // namespace Board