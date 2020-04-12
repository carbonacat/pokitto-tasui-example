#ifndef GAME_POOPSPIRIT_HPP
#   define GAME_POOPSPIRIT_HPP


#   include <Pokitto.h>
#   include "sprites.h"
#   include "Vector2.hpp"


namespace game
{
    class PoopSpirit
    {
    public: // Lifecycle.
        // Makes the spirit appear. It'll then cycle to Idle.
        void appear() noexcept;
    
        // Makes the spirit disappear.
        void disappear() noexcept;
        
        // Renders the spirit to the screen.
        void render(const Vector2& cameraPosition, bool withTP) noexcept;
        
    
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
    

    private: // Fields
        Vector2 _position {0, 0};
        int _tpTimer = 0;

        static constexpr Vector2 _spriteOrigin {8, 8};
        
        Sprite _sprite;
        
        void _updateSpeedWithCurrentTile() noexcept;
    };
}


#endif // GAME_POOPSPIRIT_HPP