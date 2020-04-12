#include <Pokitto.h>
#include "game/Game.hpp"

int main()
{
    using PC = Pokitto::Core;
    using Game = game::Game;

    Game::init();
    while (PC::isRunning())
    {
        if (!PC::update())
            continue;
        Game::update();
        Game::render();
    }

    return 0;
}