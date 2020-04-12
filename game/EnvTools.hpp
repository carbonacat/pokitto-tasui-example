#ifndef GAME_ENVTOOLS_HPP
#   define GAME_ENVTOOLS_HPP

#   include <Pokitto.h>
#   include <Tilemap.hpp>
#   include "maps.h"
#   include "Vector2.hpp"


namespace game
{
    struct EnvTools
    {
        // Returns the tile for the given coordinates.
        static inline auto tileAtPosition(const Vector2& position) noexcept
        {
            return gardenPathEnum(position.x / PROJ_TILE_W, position.y / PROJ_TILE_H);
        }
        
        static inline void loadGarden(Tilemap& tilemap) noexcept
        {
            tilemap.set(gardenPath[0], gardenPath[1], gardenPath + 2);
            for (int i = 0; i < sizeof(tiles) / (POK_TILE_W * POK_TILE_H); i++)
                tilemap.setTile(i, POK_TILE_W, POK_TILE_H, tiles + i * POK_TILE_W * POK_TILE_H);
        }
        
        static inline void loadBlack(Tilemap& tilemap) noexcept
        {
            static uint8_t emptyTilemap = 0;
            
            tilemap.set(1, 1, &emptyTilemap);
            tilemap.setTile(0, 0, 0, nullptr);
        }
    };
}


#endif // GAME_ENVTOOLS_HPP