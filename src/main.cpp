#include <cassert>
#include <stdio.h>

#include "board.hpp"

#include "render/render.hpp"
#include "render/audio.hpp"

int main(int argc, char **argv)
{
    printf("Hello World\n");

    assert(Audio::System::init() == 0);

    Audio::Sound check{"./sounds/move.wav"};
    Audio::Music fight{"./sounds/music_fight.mp3"};
    Audio::Music bass{"./sounds/music_bass.mp3"};

    fight.setVolume(MIX_MAX_VOLUME);
    bass.setVolume(MIX_MAX_VOLUME / 8);
    bass.play();

    Board::Board b;

    b.lastMove[0] = 3;
    b.lastMove[1] = 4;

    Board::setPiece(&b, 17, Pieces::QUEEN & 0x7f);
    Board::setPiece(&b, 63, Pieces::KING & 0x7f);
    Board::setPiece(&b, 6, Pieces::KNIGHT & 0x7f);

    for (uint i = 0; i < 64; i++)
    {
        printf("[%d]", b.tiles[i]);
        if (i % 8 == 0)
            printf("\n");
    }
    printf("\n");

    Render render{
        "./textures/board.png",
        "./textures/pieces.png",
        "./textures/hover.png",
        Pieces::Colour::WHITE};

    int counter = 0;

    int switchTime = SDL_GetTicks() + 5000;

    while (render.quit() == false)
    {
        if (SDL_GetTicks() >= switchTime && SDL_GetTicks() <= switchTime + 50)
        {
            printf("Switching songs\n");
            fight.play();
        }
        render.draw(&b);
        render.pollEvents();
    }

    printf("Loop over\n");

    Audio::System::quit();

    return 0;
}