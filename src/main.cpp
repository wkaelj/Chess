#include <cassert>
#include <stdio.h>

#include "board.hpp"

#include "render/render.hpp"
#include "render/audio.hpp"

int main(int argc, char **argv)
{
    assert(Audio::System::init() == 0);

    Audio::Sound move{"./sounds/move.wav"};
    Audio::Sound capture{"./sounds/capture.wav"};
    Audio::Music fight{"./sounds/music_fight.mp3"};
    Audio::Music bass{"./sounds/music_bass.mp3"};

    fight.setVolume(MIX_MAX_VOLUME);
    bass.setVolume(MIX_MAX_VOLUME / 8);
    // bass.play();

    Board::Board b = Board::createBoard(&move, &capture, NULL);

    Board::setPiece(&b, 17, Pieces::ROOK & 0x7f);
    Board::setPiece(&b, 63, Pieces::KING & 0x7f);
    Board::setPiece(&b, 6, Pieces::KNIGHT | 0x80);

    Render render{
        "/home/kael/Code/Chess/textures/board.png",
        "/home/kael/Code/Chess/textures/pieces.png",
        "/home/kael/Code/Chess/textures/hover.png",
        "/home/kael/Code/Chess/textures/legal_move.png",
        Pieces::Colour::WHITE};

    while (render.quit() == false)
    {
        render.draw(&b);
        render.pollEvents();
    }

    printf("Loop over\n");

    Audio::System::quit();

    return 0;
}