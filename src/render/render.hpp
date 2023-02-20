#pragma once

// Render a chessboard

#include <cstdint>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "../board.hpp"
#include "../moves.hpp"

#include <vector>

class Render
{
  public:
    Render(
        const char *boardTexture,
        const char *pieceTexture,
        const char *hoverTexture,
        const char *legalMoveTexture,
        Pieces::Colour colour);
    ~Render();

    void draw(Board::Board *board);
    void setPlayerColour(Pieces::Colour c) { playerColour = c; }
    Pieces::Colour getPlayerColour() { return playerColour; }

    void message(const char *messege);
    enum RenderEvent
    {
        QUIT,
        RESIZE,
        MOUSE_DOWN,
        MOUSE_MOVE,
        MOUSE_UP,
    };

    // check if the game should attempt to quit
    bool quit() { return shouldQuit; }

    void pollEvents();

  private:
    SDL_Rect getPieceSrcRect(Piece p);
    SDL_Rect getPieceDestRect(SDL_Rect *boardRect, uint8_t index);
    SDL_Rect calculateBoardRect();
    void calculatePixelScale();
    uint8_t getMouseTile(); // get the tile the mouse is over

    SDL_Window *window;
    SDL_Renderer *render;
    Pieces::Colour playerColour;

    enum ButtonState
    {
        PRESSED,
        RELEASED,
        DOWN,
        UP,
    };

    int mouseCoordinate[2];
    const static size_t MOUSE_AVERAGE_SIZE = 50;
    int mouseAverage[MOUSE_AVERAGE_SIZE]; // average position of the mouse x
                                          // axis
    size_t mouseAverageIndex;
    ButtonState rmb, lmb; // true if button pressed last frame

    int pixelScale; // the amount the hdpi monitor scales for pixels

    Piece hoveredPiece;
    uint8_t hoveredTile; // the tile the hovered piece was on

    Board::Position legalMoves[64];
    size_t legalMoveCount;

    bool shouldQuit;
    bool audio;

    SDL_Rect boardRect;

    // textures
    struct
    {
        SDL_Texture *board;
        SDL_Texture *pieces;
        SDL_Texture *hover;
        SDL_Texture *legalMove;
    } textures;
};