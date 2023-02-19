#include "render.hpp"

#include <cassert>
#include <cmath>

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_audio.h>
#include "sdl_system.hpp"

Render::Render(
    const char *boardTexture,
    const char *pieceTexture,
    const char *hoverTexture,
    Pieces::Colour colour)
{
    assert(boardTexture);
    assert(pieceTexture);
    mouseCoordinate[0] = 0, mouseCoordinate[1] = 0;
    playerColour = colour;
    lmb = UP, rmb = UP;
    shouldQuit   = false;
    audio        = true;
    hoveredPiece = Pieces::BLANK;

    if (SDL_System::initSDL(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize SDL! Error: %s\n", SDL_GetError());
        return;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("Failed to initialize SDL Image! Error %s\n", IMG_GetError());
        return;
    }

    window = SDL_CreateWindow(
        "Chess",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        printf("Failed to create SDL window! Error: %s\n", SDL_GetError());
        return;
    }

    render = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if (render == NULL)
    {
        printf("Failed to create SDL render! Error: %s\n", SDL_GetError());
        return;
    }
    SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

    // load piece textures
    textures.board  = IMG_LoadTexture(render, boardTexture);
    textures.pieces = IMG_LoadTexture(render, pieceTexture);
    textures.hover  = IMG_LoadTexture(render, hoverTexture);
    assert(textures.board && textures.pieces);

    // SDL_RenderPresent(render);
    calculatePixelScale();
    boardRect = calculateBoardRect();
}

Render::~Render()
{
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    IMG_Quit();
}

#define array_length(array) (sizeof(array) / sizeof(array[0]))

void Render::draw(Board::Board *b)
{

    assert(b);

    SDL_RenderClear(render);

    // draw board
    SDL_RenderCopy(render, textures.board, NULL, &boardRect);

    // highlight lastmove
    if (b->lastMove[0] < 64)
    {
        SDL_Rect lastmoveRects[2] = {
            getPieceDestRect(&boardRect, b->lastMove[0]),
            getPieceDestRect(&boardRect, b->lastMove[1])};

        SDL_SetRenderDrawColor(render, 0x1f, 0xff, 0x00, 0x80);
        SDL_RenderFillRects(render, lastmoveRects, 2);
    }

    // highlight mouse hover
    uint mouseTile = getMouseTile();
    if (mouseTile < 64)
    {
        SDL_Rect mouseRect = getPieceDestRect(&boardRect, mouseTile);
        SDL_RenderCopy(render, textures.hover, NULL, &mouseRect);
    }

    // draw pieces
    for (uint8_t i = 0; i < array_length(b->tiles); i++)
    {
        // draw the board in reverse order if the player is black
        Piece p = playerColour == Pieces::WHITE ? Board::getPiece(b, i)
                                                : Board::getPiece(b, 63 - i);
        if ((p & 0x7f) != Pieces::BLANK)
        {
            SDL_Rect destRect = getPieceDestRect(&boardRect, i);
            SDL_Rect srcRect  = getPieceSrcRect(p);
            if (hoveredPiece != Pieces::BLANK && i == hoveredTile)
                SDL_SetTextureAlphaMod(textures.pieces, 0x80);
            SDL_RenderCopy(render, textures.pieces, &srcRect, &destRect);
            if (hoveredPiece != Pieces::BLANK && i == hoveredTile)
                SDL_SetTextureAlphaMod(textures.pieces, 0xff);
        }
    }

    // draw hovered piece
    if (mouseTile < 64 && lmb == PRESSED)
    {
        hoveredPiece = Board::getPiece(b, mouseTile);
        hoveredTile  = mouseTile;
    }

    if ((hoveredPiece & 0x7f) != Pieces::BLANK &&
        (lmb == PRESSED || lmb == DOWN))
    {
        SDL_Rect dragPieceRect = {
            .x = mouseCoordinate[0] * pixelScale - boardRect.w / 12,
            .y = mouseCoordinate[1] * pixelScale - boardRect.w / 12,
            .w = boardRect.w / 6,
            .h = boardRect.h / 6,
        };
        SDL_Rect pieceRect = getPieceSrcRect(hoveredPiece);
        SDL_RenderCopy(render, textures.pieces, &pieceRect, &dragPieceRect);
    }

    // report move attempt
    if (lmb == RELEASED && mouseTile < 64 && hoveredPiece != Pieces::BLANK)
    {
        Board::movePiece(b, hoveredTile, mouseTile);
    }

    const uint8_t background = 0x0f;

    SDL_SetRenderDrawColor(render, background, background, background, 0xff);

    SDL_RenderPresent(render);
}

void Render::pollEvents()
{
    // if it was just pressed, it updates to down. if down, remain down
    lmb = lmb == PRESSED || lmb == DOWN ? DOWN : UP;
    rmb = rmb == PRESSED || rmb == DOWN ? DOWN : UP;
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            shouldQuit = true;
            printf("QUIT event\n");
            break;
        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                boardRect = calculateBoardRect();
                break;
            case SDL_WINDOWEVENT_DISPLAY_CHANGED: calculatePixelScale(); break;
            default: break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (e.button.button == SDL_BUTTON_LEFT)
                lmb = PRESSED;
            if (e.button.button == SDL_BUTTON_RIGHT)
                rmb = PRESSED;
            break;
        case SDL_MOUSEBUTTONUP:
            if (e.button.button == SDL_BUTTON_LEFT)
                lmb = RELEASED;
            if (e.button.button == SDL_BUTTON_RIGHT)
                rmb = RELEASED;
            break;
        case SDL_MOUSEMOTION:
            mouseCoordinate[0] = e.motion.x;
            mouseCoordinate[1] = e.motion.y;
            break;
        default: break;
        }
    }
}

void Render::message(const char *messege) { printf("%s", messege); }

SDL_Rect Render::getPieceSrcRect(Piece p)
{
    assert((p & 0x7f) != Pieces::PIECE_MAX && (p & 0x7f) != Pieces::BLANK);

    int textureSize[2];
    SDL_QueryTexture(
        textures.pieces, NULL, NULL, &textureSize[0], &textureSize[1]);
    int x           = 0;
    const int sixth = textureSize[0] / 6;
    switch (p & 0x7f)
    {
    case Pieces::BLANK: x = 0; break;
    case Pieces::PAWN: x = 5 * sixth; break;
    case Pieces::PAWN_EN_PASSANT: x = 5 * sixth; break;
    case Pieces::KNIGHT: x = 3 * sixth; break;
    case Pieces::BISHOP: x = 3 * 6; break;
    case Pieces::ROOK: x = 4 * sixth; break;
    case Pieces::ROOK_CASTLE: x = 4 * sixth; break;
    case Pieces::QUEEN: x = 1 * sixth; break;
    case Pieces::KING: x = 0; break;
    case Pieces::KING_CASTLE: x = 0; break;
    case Pieces::PIECE_MAX: assert(0 && "Piece max should not be used");
    };

    int y = Pieces::getColour(p) == Pieces::BLACK ? textureSize[1] / 2 : 0;

    SDL_Rect srcRect = {
        .x = x,
        .y = y,
        .w = sixth,
        .h = textureSize[1] / 2,
    };

    if ((p & 0x7f) == Pieces::BLANK)
    {
        srcRect.w = 1;
        srcRect.h = 1;
    }

    return srcRect;
}

SDL_Rect Render::getPieceDestRect(SDL_Rect *boardRect, uint8_t index)
{
    float w = boardRect->w / 8.f;
    float h = boardRect->h / 8.f;

    float x      = index % 8;
    float y      = index >> 3;
    float eighth = boardRect->w / 8.f;
    x *= eighth;
    y *= eighth;

    return (SDL_Rect){
        .x = (int)roundf(x) + boardRect->x,
        .y = (int)roundf(y) + boardRect->y,
        .w = (int)roundf(w),
        .h = (int)roundf(h),
    };
}

SDL_Rect Render::calculateBoardRect()
{
    int windowSize[2];
    SDL_GetRendererOutputSize(render, &windowSize[0], &windowSize[1]);

    // draw board
    int min = windowSize[0] < windowSize[1] ? windowSize[0] : windowSize[1];

    SDL_Rect boardRect;
    if (min == windowSize[0])
    {
        boardRect = {
            .x = 0,
            .y = (windowSize[1] - min) / 2,
            .w = min,
            .h = min,
        };
    }
    else
    {
        boardRect = {
            .x = (windowSize[0] - min) / 2,
            .y = 0,
            .w = min,
            .h = min,
        };
    }

    return boardRect;
}

void Render::calculatePixelScale()
{
    int renderWidth, windowWidth;
    SDL_GetWindowSize(window, &windowWidth, NULL);
    SDL_GetRendererOutputSize(render, &renderWidth, NULL);

    pixelScale = renderWidth / windowWidth;
    printf("Pixel scale = %d\n", pixelScale);
}

uint8_t Render::getMouseTile()
{
    assert(boardRect.w == boardRect.h);
    int x = mouseCoordinate[0] * pixelScale,
        y = mouseCoordinate[1] * pixelScale;

    x -= boardRect.x;
    y -= boardRect.y;
    if (x <= 2 || y <= 2 || x > boardRect.w - 2 || y > boardRect.h - 2)
        return UINT8_MAX;

    uint tilesize = boardRect.w / 8;

    y /= tilesize;
    x /= tilesize;
    uint index = x + y * 8;

    return index;
}