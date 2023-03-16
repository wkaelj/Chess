#include "board.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

// board index 0 is bottom left, index 64 is top right,
// goes horizontal

Board createBoard()
{
    Board b;
    for (uint8_t i = 0; i < 64; i++)
        b.tiles[i] = PIECE_BLANK;
    b.lastMove[0] = UINT8_MAX;
    b.lastMove[1] = UINT8_MAX;

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
    assert(piece < PIECE_PIECE_MAX);

    b->tiles[p] = piece;
}

void movePiece(Board *b, Position initial, Position final)
{
    setPiece(b, final, getPiece(b, initial)); // set final square to initial
    setPiece(b, initial, PIECE_BLANK);        // set initial square to blank
}

int generateChecksum(Board *b) { return 0; }

Piece getPieceFromChar(char c)
{
    switch (c)
    {
    case 'p': return PIECE_PAWN | 0x80;
    case 'n': return PIECE_KNIGHT | 0x80;
    case 'b': return PIECE_BISHOP | 0x80;
    case 'r': return PIECE_ROOK | 0x80;
    case 'q': return PIECE_QUEEN | 0x80;
    case 'k': return PIECE_KING | 0x80;
    case 'P': return PIECE_PAWN & 0x7f;
    case 'N': return PIECE_KNIGHT & 0x7f;
    case 'B': return PIECE_BISHOP & 0x7f;
    case 'R': return PIECE_ROOK & 0x7f;
    case 'Q': return PIECE_QUEEN & 0x7f;
    case 'K': return PIECE_KING & 0x7f;
    }
    printf("Invalid piece '%c'", c);
    return PIECE_BLANK;
}
void loadPosition(Board *b, const char *fen)
{
    // set board to blank
    for (size_t i = 0; i < 64; i++)
        b->tiles[i] = PIECE_BLANK;
    size_t len = strlen(fen);
    uint8_t x = 0, y = 7;

    size_t field = 0; // < 6
    for (size_t i = 0; i < len; i++)
    {
        switch (fen[i])
        {
        case ' ':
            return;
            field++;
            break;
        case '/':
            // move to next line
            x = 0;
            y--;
            break;
        //     break;
        case '-': break;
        case 'w':
            if (field == 1)
                0; // whites turn
            break;
        case 'b':
            if (field == 1)
            {
                0; // whites turn;
                break;
            }

        default:

            // castling
            if (field == 2)
            {
                switch (fen[i])
                {
                case 'k': b->w_castle_k = true; break;
                case 'q': b->w_castle_q = true; break;
                case 'K': b->b_castle_k = true; break;
                case 'Q': b->b_castle_q = true; break;
                }
                break;
            }

            // moves
            Piece p;
            if (field == 0)
            {
                if (isdigit(fen[i]))
                {
                    x += (fen[i] - '0');
                }
                else if (((p = getPieceFromChar(fen[i])) & 0x7f) != PIECE_BLANK)
                {
                    b->tiles[y * 8 + x] = p;
                    x++;
                    // assert(x < 8 && "Invalid FEN");
                }
                else
                {
                    assert(0 && "Invalid FEN");
                }
            }
            break;
        }
    }

    // Initialize tiles array to 0
    //     memset(b->tiles, 0, sizeof(b->tiles));

    //     int i = 0, j = 0;
    //     while (*fen != '\0')
    //     {
    //         if (*fen == '/')
    //         {
    //             i++;
    //             j = 0;
    //         }
    //         else if (isdigit(*fen))
    //         {
    //             j += (*fen - '0');
    //         }
    //         else if (*fen == ' ')
    //         {
    //             break;
    //         }
    //         else
    //         {
    //             int piece = getPieceFromChar(*fen);
    //             if (piece != -1)
    //             {
    //                 b->tiles[i * 8 + j] = piece;
    //                 j++;
    //             }
    //         }
    //         fen++;
    //     }
}