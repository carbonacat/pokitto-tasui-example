#include "game/Game.hpp"

#   include <miloslav.h>
#   include "sprites.h"
#   include "Smile.h"
#   include "ToiletPaper.h"
#   include "game/Player.hpp"
#   include "game/EnvTools.hpp"
#   include <tasui>
#   include <puits_UltimateUtopia.h>


namespace game
{
    class Game::Toolbox
    {
    public: // Constants & Types.
        using PUI = Pokitto::UI;
            
        static constexpr int dialogMargin = 2;
        static constexpr int dialogHeight = 5;
        
        static constexpr int frameToLimitDivider = 4;
        static constexpr int frameToNextBlinkDivider = 16;
        static constexpr int frameToTileDivider = 1;
        static constexpr int frameCountUntilTransitionIsDone = frameToTileDivider * (PUI::mapColumns + PUI::mapRows + 1);
            
        static constexpr Vector2 dialogFirst = {dialogMargin, dialogMargin};
        static constexpr Vector2 dialogLast = {PUI::mapColumns - 1 - dialogMargin, dialogMargin + dialogHeight};
        static constexpr unsigned pureBlackColor = 255; // Flashy pink is once again sacrificed.
        
        struct UIVariants
        {
            static constexpr unsigned standard = 0;
            static constexpr unsigned darkBG = 8;
            static constexpr unsigned halfTransparentDarkBG = 16;
        };
        
        static void setupVariants() noexcept
        {
            for (int colorI = 0; colorI < 8; colorI++)
            {
                PUI::mapColor(UIVariants::darkBG + colorI, colorI);
                PUI::mapColor(UIVariants::halfTransparentDarkBG + colorI, colorI);
            }
            // Using 0 as a remapped color will make it transparent.
            PUI::mapColor(UIVariants::darkBG + puits::UltimateUtopia::Colors::bg1, pureBlackColor);
            PUI::mapColor(UIVariants::darkBG + puits::UltimateUtopia::Colors::bg2, pureBlackColor);
            PUI::mapColor(UIVariants::halfTransparentDarkBG + puits::UltimateUtopia::Colors::bg1, 0);
            PUI::mapColor(UIVariants::halfTransparentDarkBG + puits::UltimateUtopia::Colors::bg2, pureBlackColor);
        }
        
    
    public: // Dialogs.
        // Handles a dialog which prints `text`.
        // - Returns true if the A button was pressed then released after the entire text was shown.
        // - Pressing B will instantly show the text.
        // - Note: the dialog isn't cleared when returning true.
        static bool onDialog(const char* text) noexcept
        {
            using PB = Pokitto::Buttons;
            using PUI = Pokitto::UI;
            
            if (_statusFrameNumber == 0)
            {
                // Clears the UI.
                PUI::clear();
                PUI::setOffset(-1, 0);
                
                // Renders the UI box, giving a margin of 1.
                PUI::drawBox(dialogFirst.x, dialogFirst.y, dialogLast.x, dialogLast.y);
                PUI::setCursorBoundingBox(dialogFirst.x + 1, dialogFirst.y + 1, dialogLast.x - 1, dialogLast.y - 1);
                
                _textLimit = 0;
                _entireTextIsShown = false;
            }
            
            if (PB::pressed(BTN_B))
            {
                _textLimit = strlen(text) + 1;
                _entireTextIsShown = true;
            }
            if ((_statusFrameNumber % frameToLimitDivider) == 0)
            {
                _entireTextIsShown = (strlen(text) < _textLimit);
                if (!_entireTextIsShown)
                    _textLimit++;
            }
            
            PUI::setCursor(dialogFirst.x + 1, dialogFirst.y + 1);
            PUI::fillRectTiles(dialogFirst.x + 1, dialogFirst.y + 1, dialogLast.x - 1, dialogLast.y - 1, ' ');
            PUI::printText(text, _textLimit);
            if (_entireTextIsShown)
            {
                auto nextState = (_statusFrameNumber / frameToNextBlinkDivider) % 2 == 0;
                
                PUI::drawSymbol(dialogLast.x - 1, dialogLast.y - 1, nextState ? PUI::Symbol::down : PUI::Symbol::space);
                if (PB::pressed(BTN_A))
                    return true;
            }
            return false;
        }
        
        // Clear the UI.
        static void clear() noexcept
        {
            using PUI = Pokitto::UI;
            
            PUI::clear();
            PUI::resetCursorBoundingBox();
        }
        
        // Use the UI to make a transition from black to the map+sprites.
        static bool onFadeInTransition() noexcept
        {
            if (_statusFrameNumber == 0)
            {
                PUI::clear(32, UIVariants::darkBG);
            }
            if (_statusFrameNumber % frameToTileDivider == 0)
            {
                auto reach = _statusFrameNumber / frameToTileDivider;
                
                for (int i = reach; i >= 0; i--)
                {
                    PUI::setTileAndDelta(i, reach - i, 32, UIVariants::halfTransparentDarkBG);
                    PUI::setTile(i - 2, reach - i, 0);
                }
            }
            return _statusFrameNumber >= frameCountUntilTransitionIsDone;
        }
        
    private:
        static inline int _textLimit = 0;
        static inline bool _entireTextIsShown = false;
    };
    
    // Current story status of the player.
    // It's a very silly story.
    // Each state is represented by an Update function.
    class Game::StoryStatuses
    {
    public: // Story Status Updates.
    
        // Initial state.
        static void updateBegin() noexcept
        {
            if (Toolbox::onDialog("Hi!\nWelcome to that TASMODE/TASUI Demo!"))
                _setStatusUpdate(StoryStatuses::updateHearTheIntroductionToTheGame);
        }
        
        static void updateHearTheIntroductionToTheGame() noexcept
        {
            if (Toolbox::onDialog("It's a simple demo about dialogs, in the form of a micro adventure.\nI suggest you try to go in front of the fountain for starters!"))
            {
                Toolbox::clear();
                _setStatusUpdate(StoryStatuses::updateTransitionToTheMap);
            }
        }
        
        static void updateTransitionToTheMap() noexcept
        {
            if (_onFirstFrame())
            {
                _player.setPosition({16, 16});
                _playerIsShown = true;
                _tilemapIsShown = true;
                _player.prepareForExploration();
            }
            if (Toolbox::onFadeInTransition())
                _setStatusUpdate(StoryStatuses::updateMustGoInFrontOfTheFountain);
        }
        // Exploration.
        // Going in FountainFront tiles and pressing A -> hearTheFountainQuest.
        // Going into GoToTitle tiles -> fadeOutToEscape.
        static void updateMustGoInFrontOfTheFountain() noexcept
        {
            _player.updateExploration();
        }
        
        // Dialog. "Please ... I need ... some ... TP ... to get ... out ... of here!".
        // Conclusion -> mustFindTP.
        static void updateHearTheFountainQuest() noexcept
        {
        }
        // Exploration.
        // Going on a TP Tile and pressing A -> gotTheLegendaryTP.
        // Going into GoToTitle tiles -> fadeOutToEscape.
        static void updateMustFindTP() noexcept
        {
        }
        
        // Dialog. "You found the TP! It feels very soft."
        // Conclusion -> gotTheLegendaryTP1.
        static void updateGotTheLegendaryTP() noexcept
        {
        }
        // Dialog. "Surely the Fountain will appreciate it."
        // Conclusion -> mustGoBackToTheFountainOrEscapeWithIt.
        static void updateGotTheLegendaryTP1() noexcept
        {
        }
        // Exploration.
        // Going in FountainFront tiles and pressing A -> giveTheTPToTheFountain.
        // Going into GoToTitle tiles -> fadeOutToEscapeWithTP.
        static void updateMustGoBackToTheFountainOrEscapeWithIt() noexcept
        {
        }
        
        // Cinematic. The TP is thrown inside the fountain. The Spirit appears out of nowhere.
        // End -> hearTheFountainThanks.
        static void updateGiveTheTPToTheFountain() noexcept
        {
        }
        // Dialog. "Thank you for the TP. I can now rest in peace... Thank you ... again!"
        // End -> seeTheSpiritGoingUp.
        static void updateHearTheFountainThanks() noexcept
        {
        }
        // Cinematic. The spirit goes up.
        // End -> fadeOutToGenerousEnding.
        static void updateSeeTheSpiritGoingUp() noexcept
        {
        }
        // Cinematic. Blackout.
        // End -> hearTheGenerousEnding.
        static void updateFadeOutToGenerousEnding() noexcept
        {
        }
        // Dialog. "You gave your only TP roll to the fountain. That was a selfless act."
        // End -> hearTheGenerousEnding2
        static void updateHearTheGenerousEnding() noexcept
        {
        }
        // Dialog. "You might less heavy of one TP roll, but your soul shines a bit more."
        // End -> hearTheGenerousEnding3
        static void updateHearTheGenerousEnding2() noexcept
        {
        }
        // Dialog. "You made the right choice."
        // End -> fin
        static void updateHearTheGenerousEnding3() noexcept
        {
        }
        
        // Cinematic. Blackout.
        // End -> hearTheEscapeWithTPEnding.
        static void updateFadeOutToEscapeWithTP() noexcept
        {
        }
        // Dialog. "You decided to escape with the TP instead of sharing it."
        // End -> hearTheEscapeWithTPEnding2.
        static void updateHearTheEscapeWithTPEnding() noexcept
        {
        }
        // Dialog. "Your greediness corrupted your soul, as you didn't even need it."
        // End -> fin
        static void updateHearTheEscapeWithTPEnding2() noexcept
        {
        }
        
        // Cinematic. Blackout.
        // End -> hearTheEscapeEnding.
        static void updateFadeOutToEscape() noexcept
        {
        }
        // Dialog. "You decided to ignore the quest. So neutral!"
        // End -> fin
        static void updateHearTheEscapeEnding() noexcept
        {
        }
        
        
        // Perpetual dialog. "The end."
        // (Final state).
        static void updateFin() noexcept
        {
        }
    };
    
    // Lifecycle
    
    void Game::init() noexcept {
        using PC = Pokitto::Core;
        using PD = Pokitto::Display;
        using PUI = Pokitto::UI;
    
        PC::begin();
        PD::loadRGBPalette(miloslav);
        PD::palette[Toolbox::pureBlackColor] = 0;
        EnvTools::loadGarden(_tilemap);
        PUI::setTilesetImage(puits::UltimateUtopia::tileSet);
        PUI::showTileMapSpritesUI();
        _setStatusUpdate(StoryStatuses::updateBegin);
        
        Toolbox::setupVariants();
    }
    
    void Game::update() {
        _statusFrameNumber++;
        _statusUpdate();
    }
    
    void Game::render() {
        // Camera is centered on the player.
        Vector2 cameraPosition
        {
            _player.position().x - LCDWIDTH / 2,
            _player.position().y - LCDHEIGHT / 2
        };
    
        if (_tilemapIsShown)
            _tilemap.draw(-cameraPosition.x, -cameraPosition.y);
        // Temp.
        //PD::drawSprite(-cameraPosition.x, -cameraPosition.y, ToiletPaper, false, false, 0);
        if (_playerIsShown)
            _player.render(cameraPosition);
    }
}