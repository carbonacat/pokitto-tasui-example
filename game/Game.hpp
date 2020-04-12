#ifndef GAME_GAME_HPP
#   define GAME_GAME_HPP

#   include <Pokitto.h>
#   include <Tilemap.hpp>
#   include "game/Player.hpp"

namespace game
{
    // Static class containing game objects.
    // It is responsible for the current game mode (cinematic, dialog, exploration).
    class Game
    {
    public: // Lifecycle.
        // Initializes the game objects and the pokitto.
        static void init() noexcept;
        
        // Updates the game's state.
        static void update() noexcept;
        
        // Renders the game objects to the screen.
        static void render() noexcept;
        
        
    private: // Game components.
        static inline bool _playerIsShown = false;
        static inline Player _player;
        static inline Tilemap _tilemap;
        
    private: // StoryStatuses.
        using UpdateFunction = void (*)() noexcept;
        
        static inline UpdateFunction _statusUpdate = nullptr;
        static inline int _statusFrameNumber = 0;
        static void _setStatusUpdate(UpdateFunction updateF) noexcept
        {
            _statusUpdate = updateF;
            _statusFrameNumber = -1;
        }
        static inline bool _onFirstFrame() noexcept
        {
            return _statusFrameNumber == 0;
        }
        
        class StoryStatuses;
        class Toolbox;
    };
}


#endif // GAME_GAME_HPP