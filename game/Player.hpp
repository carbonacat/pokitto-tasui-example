#ifndef GAME_PLAYER_HPP
#   define GAME_PLAYER_HPP

#   include <Pokitto.h>
#   include "sprites.h"
#   include "Vector2.hpp"


namespace game
{
    class Player
    {
    public: // Lifecycle.
        void prepareForExploration() noexcept;
    
        // Updates the player, allowing them to explore the map.
        void updateExploration() noexcept;
        
        // Renders the Player to the screen.
        void render(const Vector2& cameraPosition) noexcept;
        
    
    public: // Info.
        // Changes the position of the Player.
        void setPosition(const Vector2& position) noexcept
        {
            _position = position;
        }
        // Returns the position of the Player relative to the World.
        const Vector2& position() const noexcept
        {
            return _position;
        }
        
        auto& sprite() noexcept
        {
            return _sprite;
        }
    

    private: // Fields
        // Various speed.
        struct Speeds
        {
            static constexpr auto regular = 2;
            static constexpr auto grass = 1;
            static constexpr auto stuck = 0;
        };
        
        Vector2 _position {0, 0};
        int _speed = Speeds::stuck;

        static constexpr Vector2 _spriteOrigin {8, 8};

        Sprite _sprite;
        
        void _updateSpeedWithCurrentTile() noexcept;
    };
}


#endif // GAME_PLAYER_HPP