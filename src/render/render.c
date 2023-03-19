#include "render.h"

#include <assert.h>
#include <math.h>

#include "render_backend.h"
#include <malloc.h>

#define alloc(type) (malloc(sizeof(type)))

#define MOUSE_AVERAGE_SIZE 50

struct BoardRender
{

    RenderWindow *window;
    Render *render;

    Colour playerColour;

    size_t mouseAverageIndex;
    int mouseAverage[MOUSE_AVERAGE_SIZE]; // average position of the mouse x

    RenderCursorState rmb, lmb; // true if button pressed last frame

    uint8_t hoveredTile; // the tile the hovered piece was on
    Piece hoveredPiece;

    size_t legalMoveCount;
    bool shouldQuit;
    Position legalMoves[32];

    RenderRect boardRect;

    // textures
    struct
    {
        RenderTexture *board;
        RenderTexture *pieces;
        RenderTexture *hover;
        RenderTexture *legalMove;
    } textures;
};

// drawing helpers
void drawBoard(BoardRender *r, Board *b, RenderRect *rect, Colour playerColour);
RenderRect getPieceSrcRect(BoardRender *r, Piece p);
RenderRect getPieceDestRect(RenderRect *boardRect, uint8_t index);
// calculate the rect for the largest square that could fit in a rectangle
RenderRect calculateBoardRect(RenderRect *frame);
uint8_t getMouseTile(Render *render, RenderRect *boardRect);

BoardRender *create_board_render(
    const char *boardTexture,
    const char *pieceTexture,
    const char *hoverTexture,
    const char *legalMoveTexture,
    Colour colour)
{
    assert(boardTexture);
    assert(pieceTexture);

    BoardRender *b       = alloc(BoardRender);
    b->hoveredPiece      = PIECE_BLANK;
    b->hoveredTile       = -1;
    b->legalMoveCount    = 0;
    b->shouldQuit        = false;
    b->rmb               = RENDER_CURSOR_UP;
    b->lmb               = RENDER_CURSOR_UP;
    b->playerColour      = colour;
    b->mouseAverageIndex = 0;

    render_init();

    b->window = render_create_window("Chess", 640, 520);
    if (b->window == NULL)
        return NULL;
    b->render = render_create_render(b->window);
    if (b->render == NULL)
        return NULL;

    // load piece textures
    b->textures.board     = render_create_texture(b->render, boardTexture);
    b->textures.pieces    = render_create_texture(b->render, pieceTexture);
    b->textures.hover     = render_create_texture(b->render, hoverTexture);
    b->textures.legalMove = render_create_texture(b->render, legalMoveTexture);
    assert(
        b->textures.board && b->textures.pieces && b->textures.hover &&
        b->textures.legalMove && "All textures must have sucessfully loaded");
    render_set_texture_alpha(b->textures.legalMove, 0x80);
    assert(b->textures.board && b->textures.pieces);

    RenderRect windowRect = {.x = 0, .y = 0};
    render_get_window_size(b->window, &windowRect.w, &windowRect.h);

    b->boardRect = calculateBoardRect(&windowRect);

    return b;
}

void destroy_board_render(BoardRender *r)
{
    // icky hack to save typing. iterates through the texture pointers
    void **textures = (void **)&r->textures.board;
    for (size_t i = 0; i < sizeof(r->textures) / sizeof(r->textures.board); i++)
    {
        render_destroy_texture((RenderTexture *)*(textures + i));
    }
    render_destroy_render(r->render);
    render_destroy_window(r->window);
    render_quit();

    free(r);
}

#define array_length(array) (sizeof(array) / sizeof(array[0]))

bool board_render_quit(const BoardRender *r) { return r->shouldQuit; }

void board_render_update(BoardRender *r)
{

    RenderRect windowRect = {.x = 0, .y = 0};

    RenderEvent e = render_poll_events(r->render);
    switch (e)
    {
    case RENDER_EVENT_NONE: break;
    case RENDER_EVENT_QUIT: r->shouldQuit = true; break;
    case RENDER_EVENT_WINDOW_RESIZE:
        render_get_window_size(r->window, &windowRect.w, &windowRect.h);

        r->boardRect = calculateBoardRect(&windowRect);
        break;
    default: break;
    }

    r->lmb = render_get_cursor_state(r->render);
}

void board_render_draw(BoardRender *render, Board *b)
{
    assert(b);

    render_clear(render->render);

    int window_w, window_h;
    render_get_window_size(render->window, &window_w, &window_h);

    size_t padding = 10;

    // padding must be even!!!
    if (padding % 2 != 0)
        padding++;

    RenderRect board1 = {
        .x = padding / 2,
        .y = padding / 2,
        .w = window_w / 2 - padding,
        .h = window_h - padding,
    };

    RenderRect board2 = {
        .x = board1.w + padding,
        .y = padding / 2,
        .w = board1.w,
        .h = board1.h,
    };
    // this shouldn't break, because board1 shouldn't be changed until after the
    // function returns
    board1 = calculateBoardRect(&board1);
    board2 = calculateBoardRect(&board2);

    drawBoard(render, b, &board1, render->playerColour);
    drawBoard(
        render,
        b,
        &board2,
        render->playerColour == COLOUR_WHITE ? COLOUR_BLACK : COLOUR_WHITE);

    const uint8_t background = 0x0f;
    render_set_colour(render->render, background, background, background, 0xff);
    render_submit(render->render);
}

void drawBoard(
    BoardRender *render, Board *b, RenderRect *boardRect, Colour playerColour)
{
    // draw board
    render_draw_texture(
        render->render, boardRect, NULL, render->textures.board, 0.f);

    int mouse_x, mouse_y;
    render_get_cursor_pos(render->render, &mouse_x, &mouse_y);

    // highlight lastmove
    if (b->lastMove[0] < 64)
    {
        RenderRect lastmoveRects[2] = {
            getPieceDestRect(boardRect, b->lastMove[0]),
            getPieceDestRect(boardRect, b->lastMove[1])};

        if (render_set_colour(render->render, 0x1f, 0xff, 0x00, 0x80) ==
            RENDER_FAILURE)
        {
            printf("Failed to set render draw colour\n");
            render_draw_rects(render->render, lastmoveRects, 2);
        }
    }

    // highlight mouse hover
    uint8_t mouseTile = getMouseTile(render->render, boardRect);
    uint8_t mousePieceTile =
        playerColour == COLOUR_BLACK ? 63 - mouseTile : mouseTile;
    if (mouseTile < 64)
    {
        RenderRect mouseRect = getPieceDestRect(boardRect, mouseTile);
        render_draw_texture(
            render->render, &mouseRect, NULL, render->textures.hover, 0.f);
    }

    assert(playerColour == COLOUR_WHITE || playerColour == COLOUR_BLACK);
    // draw pieces
    for (uint8_t i = 0; i < array_length(b->tiles); i++)
    {
        // draw the board in reverse order if the player is black
        Piece p =
            playerColour == COLOUR_WHITE ? getPiece(b, i) : getPiece(b, 63 - i);
        if ((p & 0x7f) != PIECE_BLANK)
        {
            RenderRect destRect = getPieceDestRect(boardRect, i);
            RenderRect srcRect  = getPieceSrcRect(render, p);

            size_t hovertile = playerColour == COLOUR_BLACK
                                   ? 63 - render->hoveredTile
                                   : render->hoveredTile;
            if (render->hoveredPiece != PIECE_BLANK && i == hovertile)
                render_set_texture_alpha(render->textures.pieces, 0x80);
            render_draw_texture(
                render->render,
                &destRect,
                &srcRect,
                render->textures.pieces,
                0);
            if (render->hoveredPiece != PIECE_BLANK && i == hovertile)
                render_set_texture_alpha(render->textures.pieces, 0xff);
        }
    }

    // get hovered piece
    if (mouseTile < 64 && render->lmb == RENDER_CURSOR_PRESSED)
    {
        render->hoveredPiece = getPiece(b, mousePieceTile);
        render->hoveredTile  = mousePieceTile;
        if (render->hoveredPiece != PIECE_BLANK)
        {
            render->legalMoveCount =
                getLegalMoves(b, mousePieceTile, render->legalMoves);
            // reset mouse average
            for (size_t i = 0; i < MOUSE_AVERAGE_SIZE; i++)
                render->mouseAverage[i] = mouse_x;
        }
    }

    // report move attempt
    if (render->lmb == RENDER_CURSOR_RELEASED && mousePieceTile < 64 &&
        render->hoveredPiece != PIECE_BLANK)
    {
        if (mousePieceTile != render->hoveredTile)
        {
            Move m = {render->hoveredTile, mousePieceTile};
            move(b, m);
        }
        render->hoveredPiece   = PIECE_BLANK;
        render->legalMoveCount = 0;
    }

    // draw legal moves
    for (size_t i = 0; i < render->legalMoveCount; i++)
    {
        // don't draw moves that are check, as they will not be allowed
        // getLegalMoves now auto removes them to avoid per frame testing
        if (render->legalMoves[i] < 64)
        {
            Position move       = render->legalMoves[i];
            RenderRect tileRect = getPieceDestRect(
                boardRect, playerColour == COLOUR_BLACK ? 63 - move : move);
            render_draw_texture(
                render->render,
                &tileRect,
                NULL,
                render->textures.legalMove,
                0.f);
        }
    }

    // draw hovered piece
    if ((render->hoveredPiece & 0x7f) != PIECE_BLANK &&
        (render->lmb == RENDER_CURSOR_PRESSED ||
         render->lmb == RENDER_CURSOR_DOWN))
    {
        RenderRect dragPieceRect = {
            .x = mouse_x - boardRect->w / 12,
            .y = mouse_y - boardRect->w / 12,
            .w = boardRect->w / 6,
            .h = boardRect->h / 6,
        };
        // calculate average mouse position
        float mouseAverageTotal = 0xf;
        for (size_t i = 0; i < MOUSE_AVERAGE_SIZE; i++)
            mouseAverageTotal += render->mouseAverage[i];
        mouseAverageTotal /= MOUSE_AVERAGE_SIZE;

        float rotation = (mouse_x - mouseAverageTotal) * 1.f;
        rotation = (90.f / (3.141592653589f / 2.f)) * atan(rotation / 32.f);

        RenderRect pieceRect = getPieceSrcRect(render, render->hoveredPiece);
        render_set_texture_alpha(render->textures.pieces, 64 * 3);
        render_draw_texture(
            render->render,
            &dragPieceRect,
            &pieceRect,
            render->textures.pieces,
            rotation);
        render_set_texture_alpha(render->textures.pieces, UINT8_MAX);

        // slowly move average back to mouse position when mouse is stopped
        if (mouseAverageTotal != mouse_x)
        {
            assert(render->mouseAverageIndex < MOUSE_AVERAGE_SIZE);
            render->mouseAverage[render->mouseAverageIndex] = mouse_x;
            render->mouseAverageIndex =
                (render->mouseAverageIndex + 1) % MOUSE_AVERAGE_SIZE;
        }
    }
}

RenderRect getPieceSrcRect(BoardRender *r, Piece p)
{
    assert((p & 0x7f) != PIECE_PIECE_MAX && (p & 0x7f) != PIECE_BLANK);

    int textureSize[2];
    render_get_texture_size(
        r->textures.pieces, &textureSize[0], &textureSize[1]);
    int x           = 0;
    const int sixth = textureSize[0] / 6;
    switch (p & 0x7f)
    {
    case PIECE_BLANK: x = 0; break;
    case PIECE_PAWN: x = 5 * sixth; break;
    case PIECE_KNIGHT: x = 3 * sixth; break;
    case PIECE_BISHOP: x = 2 * sixth; break;
    case PIECE_ROOK: x = 4 * sixth; break;
    case PIECE_QUEEN: x = 1 * sixth; break;
    case PIECE_KING: x = 0; break;
    case PIECE_PIECE_MAX: assert(0 && "Piece max should not be used");
    };

    int y = getColour(p) == COLOUR_BLACK ? textureSize[1] / 2 : 0;

    RenderRect srcRect = {
        .x = x,
        .y = y,
        .w = sixth,
        .h = textureSize[1] / 2,
    };

    if ((p & 0x7f) == PIECE_BLANK)
    {
        srcRect.w = 1;
        srcRect.h = 1;
    }

    return srcRect;
}

RenderRect getPieceDestRect(RenderRect *boardRect, uint8_t index)
{
    float w = boardRect->w / 8.f;
    float h = boardRect->h / 8.f;

    float x      = index % 8;
    float y      = index >> 3;
    float eighth = boardRect->w / 8.f;
    x *= eighth;
    y *= eighth;

    return (RenderRect){
        .x = (int)roundf(x) + boardRect->x,
        .y = (int)roundf(y) + boardRect->y,
        .w = (int)roundf(w),
        .h = (int)roundf(h),
    };
}

RenderRect calculateBoardRect(RenderRect *frame)
{
    int windowSize[2] = {frame->w, frame->h};

    // draw board
    int min = windowSize[0] < windowSize[1] ? windowSize[0] : windowSize[1];

    RenderRect boardRect;
    if (min == windowSize[0])
    {
        boardRect = (RenderRect){
            .x = 0 + frame->x,
            .y = (windowSize[1] - min) / 2 + frame->y,
            .w = min,
            .h = min,
        };
    }
    else
    {
        boardRect = (RenderRect){
            .x = (windowSize[0] - min) / 2 + frame->x,
            .y = 0 + frame->y,
            .w = min,
            .h = min,
        };
    }

    return boardRect;
}

uint8_t getMouseTile(Render *render, RenderRect *boardRect)
{
    int mouse_x, mouse_y;
    render_get_cursor_pos(render, &mouse_x, &mouse_y);
    assert(boardRect->w == boardRect->h);
    int x = mouse_x, y = mouse_y;

    x -= boardRect->x;
    y -= boardRect->y;
    if (x <= 2 || y <= 2 || x > boardRect->w - 2 || y > boardRect->h - 2)
        return UINT8_MAX;

    unsigned tilesize = boardRect->w / 8;

    y /= tilesize;
    x /= tilesize;
    unsigned index = x + y * 8;

    return index;
}