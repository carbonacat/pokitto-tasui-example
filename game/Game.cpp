#include "game/Game.hpp"

#include <miloslav.h>
#include "sprites.h"
#include "game/Player.hpp"
#include "game/EnvTools.hpp"
#include <tasui>
#include <puits_UltimateUtopia.h>
#include <puits_UltimateUtopia4.h>


namespace game
{
    class Game::Toolbox
    {
    public: // Constants & Types.
        using PUI = Pokitto::UI;
            
        static constexpr int dialogMargin = 2;
        static constexpr int dialogHeight = 5;
        
        static constexpr int frameToLimitDivider = 2;
        static constexpr int frameToFloatingLimitDivider = 16;
        static constexpr int frameToNextBlinkDivider = 16;
        static constexpr int frameToTileDivider = 1;
        static constexpr int frameCountUntilTransitionIsDone = frameToTileDivider * (PUI::mapColumns + PUI::mapRows + 1);
            
        static constexpr Vector2 dialogFirst = {dialogMargin, dialogMargin};
        static constexpr Vector2 dialogLast = {PUI::mapColumns - 1 - dialogMargin, dialogMargin + dialogHeight};
        static constexpr unsigned pureBlackColor = 255; // Flashy pink is once again sacrificed.
        
        struct UIVariants
        {
            static constexpr unsigned standard = 0;
            static constexpr unsigned blackBG = 8;
            static constexpr unsigned halfBlackBG = 16;
            static constexpr unsigned red = 24;
            static constexpr unsigned green = 32;
        };
        
        static void setupVariantsForUltimateUtopia() noexcept
        {
            for (int colorI = 0; colorI < 8; colorI++)
            {
                PUI::mapColor(UIVariants::blackBG + colorI, colorI);
                PUI::mapColor(UIVariants::halfBlackBG + colorI, colorI);
                // Red/Green keeps the standard's background, but will have variants of red/green otherwise.
                PUI::mapColor(UIVariants::red + colorI, 168 + colorI);
                PUI::mapColor(UIVariants::green + colorI, 192 + colorI);
            }
            // Using 0 as a remapped color will make it transparent.
            PUI::mapColor(UIVariants::blackBG + puits::UltimateUtopia::Colors::bg1, pureBlackColor);
            PUI::mapColor(UIVariants::blackBG + puits::UltimateUtopia::Colors::bg2, pureBlackColor);
            PUI::mapColor(UIVariants::halfBlackBG + puits::UltimateUtopia::Colors::bg1, 0);
            PUI::mapColor(UIVariants::halfBlackBG + puits::UltimateUtopia::Colors::bg2, pureBlackColor);
            // Red/Green keeps the standard's background, but will have variants of red/green otherwise.
            PUI::mapColor(UIVariants::red + puits::UltimateUtopia::Colors::outside, puits::UltimateUtopia::Colors::outside);
            PUI::mapColor(UIVariants::red + puits::UltimateUtopia::Colors::bg1, puits::UltimateUtopia::Colors::bg1);
            PUI::mapColor(UIVariants::red + puits::UltimateUtopia::Colors::bg2, puits::UltimateUtopia::Colors::bg2);
            PUI::mapColor(UIVariants::green + puits::UltimateUtopia::Colors::outside, puits::UltimateUtopia::Colors::outside);
            PUI::mapColor(UIVariants::green + puits::UltimateUtopia::Colors::bg1, puits::UltimateUtopia::Colors::bg1);
            PUI::mapColor(UIVariants::green + puits::UltimateUtopia::Colors::bg2, puits::UltimateUtopia::Colors::bg2);
            
            PUI::setTilesetImage(puits::UltimateUtopia::tileSet);
        }
        
        static void setupVariantsForUltimateUtopia4() noexcept
        {
            constexpr int defaultColors[4] = {0, 11, 90, 6};
            
            for (int colorI = 0; colorI < 4; colorI++)
            {
                PUI::mapColor(UIVariants::standard + colorI, defaultColors[colorI]);
                PUI::mapColor(UIVariants::blackBG + colorI, defaultColors[colorI]);
                PUI::mapColor(UIVariants::halfBlackBG + colorI, defaultColors[colorI]);
                PUI::mapColor(UIVariants::red + colorI, defaultColors[colorI]);
                PUI::mapColor(UIVariants::green + colorI, defaultColors[colorI]);
            }
            // Using 0 as a remapped color will make it transparent.
            PUI::mapColor(UIVariants::blackBG + puits::UltimateUtopia4::Colors::bg, pureBlackColor);
            PUI::mapColor(UIVariants::halfBlackBG + puits::UltimateUtopia4::Colors::bg, 0);
            // Red/Green keeps the standard's background, but will have variants of red/green otherwise.
            PUI::mapColor(UIVariants::red + puits::UltimateUtopia4::Colors::main, 174);
            PUI::mapColor(UIVariants::green + puits::UltimateUtopia4::Colors::main, 204);
        
            PUI::setTilesetImage(puits::UltimateUtopia4::tileSet);
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
                PUI::setCursorDelta(UIVariants::standard);
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
            _textLimit = 0;
            _entireTextIsShown = false;
            _hintTimer = 0;
        }
        
        
    private: // Hacking Sequence.
        static inline struct Hacking
        {
            using ButtonSequence = std::array<std::uint8_t, 15>;
            
            static constexpr auto gaugeFirst = dialogFirst + Vector2{1, 3};
            static constexpr auto gaugeLast = Vector2{dialogLast.x - 1, gaugeFirst.y};
            static constexpr auto sequenceFirst = Vector2{dialogFirst.x + 2, dialogFirst.y + 4};
            static constexpr auto sequenceLast = Vector2{dialogLast.x - 2, dialogFirst.y + 4};

            // General progress.
            int progress;
            // If positive, the number of frame the input is locked.
            int remainingLockedFrames;
            // Nice trick from tuxinator2009.
            std::uint8_t previousButtonsStates;
            // The sequence to follow
            ButtonSequence sequence;
            // Where we are in the sequence.
            unsigned sequenceIndex;
        } hacking {};
        
        static std::uint8_t indexForButton(std::uint8_t buttonId) noexcept
        {
            using PB = Pokitto::UI;
            
            switch (buttonId)
            {
            case LEFTBIT:
                return PUI::indexForSymbol(PUI::Symbol::left);
            case RIGHTBIT:
                return PUI::indexForSymbol(PUI::Symbol::right);
            case UPBIT:
                return PUI::indexForSymbol(PUI::Symbol::up);
            case DOWNBIT:
                return PUI::indexForSymbol(PUI::Symbol::down);
            case ABIT:
                return 'A';
            case BBIT:
                return 'B';
            case CBIT:
                return 'C';
            }
            return '?';
        }
        
        static void generateAndRenderSequence(int sequenceCount) noexcept
        {
            for (int sequenceI = 0; sequenceI < sequenceCount; sequenceI++)
            {
                auto sequenceButton = rand() % NUM_BTN; // all buttons!
                
                hacking.sequence[sequenceI] = sequenceButton;
                PUI::setTileAndDelta(Hacking::sequenceFirst.x + sequenceI * 2, Hacking::sequenceFirst.y, indexForButton(sequenceButton), UIVariants::standard);
                if (sequenceI + 1 < sequenceCount)
                    PUI::setTileAndDelta(Hacking::sequenceFirst.x + sequenceI * 2 + 1, Hacking::sequenceFirst.y, '-', UIVariants::standard);
            }
        }

    
    public: // Hacking Sequence.
        enum struct HackingResult
        {
            progress,
            failed,
            success
        };
        
        // Initiates and conducts the Hacking Sequence.
        // - A sequence of `sequenceCount` will be generated.
        // - If the Progress gets over `progressForSuccess`, it succeded.
        // - If the Progress reaches 0, it failed.
        // - Every each `framePerDecrementTick` frames, the Progress will be lowered by `progressDecrement`.
        // - When pressing the right button, the generated sequence will progress.
        // - When finishing a sequence, `progressIncrementIfSequenceDone` will be added to the Progress.
        // - When pressing the wrong button, the sequence will be reset and `lockedFramesIfWrong` frames will be waited until being able to enter again a sequence.
        // - Not compatible with the Dialog.
        // - Big up to Torbuntu!
        static HackingResult onHacking(const char* title,
                                       int initialProgress, int progressForSuccess,
                                       int progressDecrement, int framePerDecrement,
                                       int sequenceCount, int progressIncrementIfSequenceDone, int lockedFramesIfWrong) noexcept
        {
            using PB = Pokitto::Buttons;
            
            if (_onFirstFrame())
            {
                // Clears the UI.
                PUI::clear();
                PUI::setOffset(-1, 0);
                
                // Renders the UI box, giving a margin of 1.
                PUI::drawBox(dialogFirst.x, dialogFirst.y, dialogLast.x, dialogLast.y);
                PUI::setCursor(dialogFirst.x + 1, dialogFirst.y + 1);
                PUI::setCursorDelta(UIVariants::standard);
                PUI::printString(title);
    
                hacking.progress = initialProgress;
                hacking.remainingLockedFrames = 0;
                
                generateAndRenderSequence(sequenceCount);
                hacking.sequenceIndex = 0;

                // Doing this here avoid previously pressed buttons (e.g. when ending a dialog).
                hacking.previousButtonsStates = PB::buttons_state;
            }
            
            // Initial check.
            if (hacking.progress <= 0)
                return HackingResult::failed;
            if (hacking.progress >= progressForSuccess)
                return HackingResult::success;
            
            
            // Updating the progress.
            {
                // Decrementing the progress at the right frame.
                if (_statusFrameNumber % framePerDecrement == 0)
                    hacking.progress -= progressDecrement;
                
                // Nice trick from tuxinator2009.
                // A button was just pressed if it's currently pressed, but wasn't previously.
                auto justPressedStates = PB::buttons_state & (~hacking.previousButtonsStates);
                // TODO: Read the expected sequence.
                auto expectedButton = hacking.sequence[hacking.sequenceIndex];
                
                if (hacking.remainingLockedFrames > 0)
                    hacking.remainingLockedFrames--;
                else if (justPressedStates)
                {
                    // Was a button just pressed?
                    if (justPressedStates & (1 << expectedButton))
                    {
                        // Good!
                        hacking.sequenceIndex++;
                        // If we finished the sequence...
                        if (hacking.sequenceIndex >= sequenceCount)
                        {
                            // We get points and a new one!
                            hacking.progress += progressIncrementIfSequenceDone;
                            generateAndRenderSequence(sequenceCount);
                            hacking.sequenceIndex = 0;
                        }
                    }
                    else
                    {
                        // Wrong...
                        hacking.remainingLockedFrames = lockedFramesIfWrong;
                        hacking.sequenceIndex = 0;
                    }
                }
                hacking.previousButtonsStates = PB::buttons_state;
            }
            
            // Rendering the gauge.
            {
                PUI::drawGauge(Hacking::gaugeFirst.x, Hacking::gaugeLast.x, Hacking::gaugeFirst.y,
                               hacking.progress, progressForSuccess);
                if (hacking.remainingLockedFrames > 0)
                {
                    PUI::fillRectDeltas(Hacking::gaugeFirst.x, Hacking::gaugeFirst.y, Hacking::gaugeLast.x, Hacking::gaugeLast.y,
                                        UIVariants::red);
                    PUI::fillRectDeltas(Hacking::sequenceFirst.x, Hacking::sequenceFirst.y, Hacking::sequenceLast.x, Hacking::sequenceLast.y,
                                        UIVariants::red);
                }
                else
                {
                    PUI::fillRectDeltas(Hacking::gaugeFirst.x, Hacking::gaugeFirst.y, Hacking::gaugeLast.x, Hacking::gaugeLast.y,
                                        UIVariants::standard);
                    PUI::fillRectDeltas(Hacking::sequenceFirst.x, Hacking::sequenceFirst.y, Hacking::sequenceFirst.x + hacking.sequenceIndex * 2 - 1, Hacking::sequenceLast.y,
                                        UIVariants::green);
                    PUI::fillRectDeltas(Hacking::sequenceFirst.x + hacking.sequenceIndex * 2 + 1, Hacking::sequenceFirst.y, Hacking::sequenceLast.x, Hacking::sequenceLast.y,
                                        UIVariants::standard);
                    PUI::setDelta(Hacking::sequenceFirst.x + hacking.sequenceIndex * 2, Hacking::sequenceFirst.y, UIVariants::blackBG);
                }
            }
            
            // Checking the end conditions.
            if (hacking.progress < 0)
                return HackingResult::failed;
            if (hacking.progress >= progressForSuccess)
                return HackingResult::success;
            return HackingResult::progress;
        }
        
        
    public: // Transitions.
        
        // Use the UI to make a transition from black to the map+sprites.
        static bool onFadeInTransition() noexcept
        {
            if (_statusFrameNumber == 0)
            {
                PUI::clear(32, UIVariants::blackBG);
            }
            if (_statusFrameNumber % frameToTileDivider == 0)
            {
                auto reach = _statusFrameNumber / frameToTileDivider;
                
                for (int i = reach; i >= 0; i--)
                {
                    PUI::setTileAndDelta(i, reach - i, 32, UIVariants::halfBlackBG);
                    PUI::setTile(i - 2, reach - i, 0);
                }
            }
            return _statusFrameNumber >= frameCountUntilTransitionIsDone;
        }
        
        // Use the UI to make a transition from black to the map+sprites.
        static bool onFadeOutTransition() noexcept
        {
            if (_statusFrameNumber == 0)
                PUI::clear();
            if (_statusFrameNumber % frameToTileDivider == 0)
            {
                auto reach = _statusFrameNumber / frameToTileDivider;
                
                for (int i = reach; i >= 0; i--)
                {
                    PUI::setTileAndDelta(i, reach - i, 32, UIVariants::halfBlackBG);
                    PUI::setTileAndDelta(i - 2, reach - i, 32, UIVariants::blackBG);
                }
            }
            return _statusFrameNumber >= frameCountUntilTransitionIsDone;
        }
        
        
    public: // Tips & Hints.
    
        // Renders a center text on the screen on a black screen, letter by letter.
        static bool onFloatingText(const char* text) noexcept
        {
            auto textLen = strlen(text);
            auto textLimit = _statusFrameNumber / frameToFloatingLimitDivider;
            
            PUI::clear(32, UIVariants::blackBG);
            PUI::resetCursorBoundingBox();
            PUI::setCursorDelta(UIVariants::blackBG);
            PUI::setCursor((PUI::mapColumns - textLen) / 2, PUI::mapRows / 2);
            PUI::printText(text, textLimit);
            return textLimit > textLen;
        }
        
        // Renders a hint with half BG, letter by letter.
        static void onHint(const char* text) noexcept
        {
            auto textLimit = _hintTimer / frameToLimitDivider;
            auto textLen = strlen(text);
            
            PUI::setCursorDelta(UIVariants::halfBlackBG);
            PUI::setCursor(1, 0);
            PUI::printText(text, textLimit);
            _hintTimer += (textLimit < textLen) ? 1 : 0;
        }
        
        // Erases the hint, letter by letter.
        static void onDehint() noexcept
        {
            _hintTimer -= (_hintTimer > 0) ? 1 : 0;
            
            auto textLimit = _hintTimer / frameToLimitDivider;
            
            PUI::fillRectTiles(1 + textLimit, 0, PUI::mapColumns - 1, 0, 0);
        }
        
    private:
        static inline int _textLimit = 0;
        static inline bool _entireTextIsShown = false;
        static inline int _hintTimer;
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
        
        static void tmpHackingOK() noexcept
        {
            if (Toolbox::onDialog("Hacking finished! I mean, you found the TP!"))
                _setStatusUpdate(StoryStatuses::updateHearTheIntroductionToTheGame);
        }
        
        static void tmpHackingFailed() noexcept
        {
            if (Toolbox::onDialog("Duuuude..."))
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
                _poopSpirit.setPosition({11*16, 3*16+8});
                _playerIsShown = true;
                EnvTools::loadGarden(_tilemap);
                _player.prepareForExploration();
            }
            if (Toolbox::onFadeInTransition())
                _setStatusUpdate(StoryStatuses::updateMustGoInFrontOfTheFountain);
        }
        
        static void updateMustGoInFrontOfTheFountain() noexcept
        {
            using PB = Pokitto::Buttons;
            
            _player.updateExploration();
            
            auto playerTile = EnvTools::tileAtPosition(_player.position());
            
            if (playerTile & GoToTitle)
                _setStatusUpdate(StoryStatuses::updateFadeOutToEscape);
            else if (playerTile & FountainFront)
            {
                Toolbox::onHint("A - Talk");
                if (PB::pressed(BTN_A))
                {
                    _player.sprite().play(dude, Dude::punchN);
                    _setStatusUpdate(StoryStatuses::updateHearTheFountainQuest);
                }
            }
            else
                Toolbox::onDehint();
        }
        
        static void updateHearTheFountainQuest() noexcept
        {
            if (_onFirstFrame())
                _poopSpirit.appear();
            if (Toolbox::onDialog("Please...\nI need... some... TP... to get... out... of here!"))
            {
                _poopSpirit.disappear();
                Toolbox::clear();
                _setStatusUpdate(StoryStatuses::updateMustFindTP);
            }
        }
        // Exploration.
        // Going on a TP Tile and pressing A -> gotTheLegendaryTP.
        // Going into GoToTitle tiles -> fadeOutToEscape.
        static void updateMustFindTP() noexcept
        {
            using PB = Pokitto::Buttons;
            
            _player.updateExploration();
            
            auto playerTile = EnvTools::tileAtPosition(_player.position());
            
            if (playerTile & GoToTitle)
                _setStatusUpdate(StoryStatuses::updateFadeOutToEscape);
            else if (playerTile & RandomItem)
            {
                Toolbox::onHint("A - Search");
                if (PB::pressed(BTN_A))
                    _setStatusUpdate(StoryStatuses::updateFindingTP);
            }
            else
                Toolbox::onDehint();
        }
        
        static void updateFindingTP() noexcept
        {
            using HackingResult = Toolbox::HackingResult;
            
            Toolbox::onDehint();
            switch (Toolbox::onHacking("Searching the Grass...",
                                       256, 1024,
                                       1, 3,
                                       8, 256, 32))
            {
            case HackingResult::failed:
                _setStatusUpdate(StoryStatuses::updateTheTPEscaped);
                break ;
            case HackingResult::progress:
                // The hacking is still going on, nothing to do.
                break ;
            case HackingResult::success:
                _player.sprite().play(dude, Dude::yay);
                _setStatusUpdate(StoryStatuses::updateGotTheLegendaryTP);
                break ;
            }
        }
        
        static void updateTheTPEscaped() noexcept
        {
            Toolbox::onDehint();
            if (Toolbox::onDialog("Unfortunately the TP fled away."))
            {
                Toolbox::clear();
                _setStatusUpdate(StoryStatuses::updateMustFindTP);
            }
        }
        static void updateGotTheLegendaryTP() noexcept
        {
            Toolbox::onDehint();
            _tpIsShown = true;
            if (Toolbox::onDialog("You found the Legendary Toilet Paper!\nIt feels very soft."))
                _setStatusUpdate(StoryStatuses::updateGotTheLegendaryTP1);
        }
        static void updateGotTheLegendaryTP1() noexcept
        {
            Toolbox::onDehint();
            if (Toolbox::onDialog("Surely the Fountain will appreciate it!"))
            {
                Toolbox::clear();
                _setStatusUpdate(StoryStatuses::updateMustGoBackToTheFountainOrEscapeWithIt);
                _tpIsShown = false;
                _player.sprite().play(dude, Dude::walkS);
            }
        }
        static void updateMustGoBackToTheFountainOrEscapeWithIt() noexcept
        {
            using PB = Pokitto::Buttons;
            
            _player.updateExploration();
            
            auto playerTile = EnvTools::tileAtPosition(_player.position());
            
            if (playerTile & GoToTitle)
                _setStatusUpdate(StoryStatuses::updateFadeOutToEscapeWithTP);
            else if (playerTile & FountainFront)
            {
                Toolbox::onHint("A - Give the TP");
                if (PB::pressed(BTN_A))
                {
                    _player.sprite().play(dude, Dude::punchN);
                    _tpIsShown = true;
                    _setStatusUpdate(StoryStatuses::updateGiveTheTPToTheFountain);
                }
            }
            else
                Toolbox::onDehint();
        }
        
        static void updateGiveTheTPToTheFountain() noexcept
        {
            if (_onFirstFrame())
                _poopSpirit.appear();
            if (_statusFrameNumber == 32)
                _setStatusUpdate(StoryStatuses::updateHearTheFountainThanks);
            Toolbox::onDehint();
        }
        static void updateHearTheFountainThanks() noexcept
        {
            Toolbox::onDehint();
            _tpIsShown = false;
            if (Toolbox::onDialog("Thank you for the TP. I can now rest in peace...\nThank you ... again!"))
            {
                Toolbox::clear();
                _setStatusUpdate(StoryStatuses::updateSeeTheSpiritGoingUp);
            }
        }
        
        static void updateSeeTheSpiritGoingUp() noexcept
        {
            if (_onFirstFrame())
                _poopSpirit.disappear();
            _poopSpirit.setPosition(_poopSpirit.position() + Vector2{0, -1});
            if (_statusFrameNumber == 32)
                _setStatusUpdate(StoryStatuses::updateFadeOutToGenerousEnding);
        }
        
        static void updateFadeOutToGenerousEnding() noexcept
        {
            if (Toolbox::onFadeOutTransition())
            {
                _setStatusUpdate(StoryStatuses::updateHearTheGenerousEnding);
                _playerIsShown = false;
                EnvTools::loadBlack(_tilemap);
            }
        }
        static void updateHearTheGenerousEnding() noexcept
        {
            if (Toolbox::onDialog("You gave your only TP roll to the fountain spirit. That was a selfless act."))
                _setStatusUpdate(StoryStatuses::updateHearTheGenerousEnding2);
        }
        static void updateHearTheGenerousEnding2() noexcept
        {
            if (Toolbox::onDialog("You might be less heavy of one TP roll, but your soul shines a bit more."))
                _setStatusUpdate(StoryStatuses::updateHearTheGenerousEnding3);
        }
        static void updateHearTheGenerousEnding3() noexcept
        {
            if (Toolbox::onDialog("You made the right choice."))
                _setStatusUpdate(StoryStatuses::updateFin);
        }
        
        static void updateFadeOutToEscapeWithTP() noexcept
        {
            if (Toolbox::onFadeOutTransition())
            {
                _setStatusUpdate(StoryStatuses::updateHearTheEscapeWithTPEnding);
                _playerIsShown = false;
                EnvTools::loadBlack(_tilemap);
            }
        }
        
        static void updateHearTheEscapeWithTPEnding() noexcept
        {
            if (Toolbox::onDialog("You decided to escape with the TP instead of sharing it."))
                _setStatusUpdate(StoryStatuses::updateHearTheEscapeWithTPEnding2);
        }
        static void updateHearTheEscapeWithTPEnding2() noexcept
        {
            if (Toolbox::onDialog("Your greediness corrupted your soul, as you didn't even need it."))
                _setStatusUpdate(StoryStatuses::updateHearTheEscapeWithTPEnding3);
        }
        static void updateHearTheEscapeWithTPEnding3() noexcept
        {
            if (Toolbox::onDialog("At least, you've got some TP to wipe yourself out..."))
                _setStatusUpdate(StoryStatuses::updateFin);
        }
        
        static void updateFadeOutToEscape() noexcept
        {
            if (Toolbox::onFadeOutTransition())
            {
                _setStatusUpdate(StoryStatuses::updateHearTheEscapeEnding);
                _playerIsShown = false;
                EnvTools::loadBlack(_tilemap);
            }
        }
        static void updateHearTheEscapeEnding() noexcept
        {
            if (Toolbox::onDialog("You decided to ignore the quest.\n So neutral!"))
                _setStatusUpdate(StoryStatuses::updateFin);
        }
        
        // (Final state).
        static void updateFin() noexcept
        {
            Toolbox::onFloatingText("The End.");
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
        PUI::showTileMapSpritesUI();
        _setStatusUpdate(StoryStatuses::updateBegin);
        
        Toolbox::setupVariantsForUltimateUtopia();
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
    
        _tilemap.draw(-cameraPosition.x, -cameraPosition.y);
        if (_playerIsShown)
        {
            _poopSpirit.render(cameraPosition, _tpForPoop);
            _player.render(cameraPosition, _tpIsShown);
        }
    }
}