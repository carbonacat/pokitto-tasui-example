#include "game/Player.hpp"

#include "game/EnvTools.hpp"
#include "ToiletPaper.h"


namespace game
{
    static constexpr Vector2 toiletPaperOrigin = {8, 8};
    
    // Lifecycle.
    void Player::prepareForExploration() noexcept
    {
        _sprite.play(dude, Dude::walkS);
    }
    
    void Player::updateExploration() noexcept
    {
        using PB = Pokitto::Buttons;
        
        auto oldPosition = _position;
        Vector2 move {
            (PB::leftBtn() ? -_speed : 0) + (PB::rightBtn() ? _speed : 0),
            (PB::upBtn() ? -_speed : 0) + (PB::downBtn() ? _speed : 0)
        };
        
        // Updating the rendered direction.
        if (move.x > 0)
        {
            if (_sprite.animation != Dude::walkE)
                _sprite.play(dude, Dude::walkE);
        }
        else if (move.x < 0)
        {
            if (_sprite.animation != Dude::walkW)
                _sprite.play(dude, Dude::walkW);
        }
        else if (move.y > 0)
        {
            if (_sprite.animation != Dude::walkS)
                _sprite.play(dude, Dude::walkS);
        }
        else if (move.y < 0)
        {
            if (_sprite.animation != Dude::walkN)
                _sprite.play(dude, Dude::walkN);
        }
    
        // Updating position separately for each axis so the Player can slide on walls.
        if (move.x != 0)
        {
            _position.x += move.x;
            if (EnvTools::tileAtPosition(_position) & Collide)
                _position.x -= move.x;
        }
        if (move.y != 0)
        {
            _position.y += move.y;
            if (EnvTools::tileAtPosition(_position) & Collide)
                _position.y -= move.y;
        }
        
        _updateSpeedWithCurrentTile();
    }
    
    void Player::render(const Vector2& cameraPosition, bool withTP) noexcept
    {
        using PD = Pokitto::Display;
        
        if (withTP)
        {
            _tpTimer += (_tpTimer < 18) ? 1 : 0;
        }
        else
            _tpTimer -= (_tpTimer > 0) ? 1 : 0;
        
        if (_tpTimer != 0)
            PD::drawSprite(_position.x - cameraPosition.x - toiletPaperOrigin.x, _position.y - cameraPosition.y - toiletPaperOrigin.y - _tpTimer, ToiletPaper, false, false, 0);
        _sprite.draw(_position.x - cameraPosition.x - _spriteOrigin.x, _position.y - cameraPosition.y - _spriteOrigin.y,
                     false, false, 0);
    }
    
    
    // Modes.
    
    void Player::_updateSpeedWithCurrentTile() noexcept
    {
        if (EnvTools::tileAtPosition(_position) & WalkOnGrass)
            _speed = Speeds::grass;
        else
            _speed = Speeds::regular;
    }
}