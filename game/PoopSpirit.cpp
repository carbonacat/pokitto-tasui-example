#include "game/PoopSpirit.hpp"

#include "game/EnvTools.hpp"
#include "ToiletPaper.h"


namespace game
{
    static constexpr Vector2 toiletPaperOrigin = {8, 8};
    
    // Lifecycle.
    void PoopSpirit::appear() noexcept
    {
        _sprite.play(poopSpirit, ::PoopSpirit::appear);
        // Chains an Idle animation.
        _sprite.onEndAnimation = +[](Sprite *sprite){ sprite->play(poopSpirit, ::PoopSpirit::idle); };
    }
    void PoopSpirit::disappear() noexcept
    {
        _sprite.play(poopSpirit, ::PoopSpirit::disappear);
        // Chains nothing.
        _sprite.onEndAnimation = +[](Sprite *sprite){ sprite->play(poopSpirit, ::PoopSpirit::disappeared); };
    }
    
    void PoopSpirit::render(const Vector2& cameraPosition, bool withTP) noexcept
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
}