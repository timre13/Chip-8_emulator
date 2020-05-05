#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <filesystem>
#include <algorithm>

//------------------------------------------------------------------------------

// This switch marks whether the result of the bit
// shifts (8XY6 and 0x8XYE instructions) should be stored in Y or in X.
// The original implementation stored the result in Y;
bool storeBitShiftResultInY{true};

// This variable marks whether the I (index register) should be
// incremented after reading from or writing
// to memory (FX55 and FX65 instructions).
// In the original implementation this does happen.
bool incrementIAfterMemoryOperation{true};

// Color of background
uint8_t bgColorR{155};
uint8_t bgColorG{250};
uint8_t bgColorB{220};

// Color of active pixels
uint8_t fgColorR{65 };
uint8_t fgColorG{150};
uint8_t fgColorb{150};

//------------------------------------------------------------------------------

#include "Chip-8.h"
#include "sdl_file_chooser.h"
#include "DoubleAsker.h"
#include "sound.h"

int main()
{
    FileChooser fileChooser{"./roms"};
    std::string romFilename{fileChooser.get()};
    
    // If the user canceled the file selection, quit.
    if (romFilename.size() == 0)
        return 0;
    
    DoubleAsker doubleAskerDialog;
    double emulationSpeed{doubleAskerDialog.get()};
    
    // If the user canceled the entering of the emulation speed, quit.
    if (emulationSpeed <= 0)
        return 0;

    std::cout << "Filename: " << romFilename << std::endl;
    std::cout << "Emulation speed: " << emulationSpeed << std::endl;

    Chip8 chip8{romFilename};
    
    std::cout << std::hex;
    
    const double frameDelay{1000.0/500/emulationSpeed};
    
    bool isRunning{true};
    bool isPaused{false};
    bool wasPaused{false};
    bool isSteppingMode{false};
    bool shouldStep{false}; // no effect when not in stepping mode
    while (isRunning && !chip8.hasEnded)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
			switch (event.type)
			{
				case SDL_QUIT:
					isRunning = false;
					break;

				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							isPaused = !isPaused;
							isSteppingMode = false;
							break;
						case SDLK_F12:
							isRunning = false;
							break;
						case SDLK_F11:
						    chip8.toggleFullscreen();
						    break;
						case SDLK_F10:
						    chip8.toggleDebugMode();
						    break;
						case SDLK_F9:
						    chip8.toggleCursor();
						    break;
						case SDLK_F6:
						    shouldStep = true;
						    break;
						case SDLK_F5:
						    isSteppingMode = !isSteppingMode;
						    isPaused = false;
						    break;
					}
					break;

				case SDL_WINDOWEVENT:
					if (event.window.windowID == chip8.getWindowID())
					{
						switch (event.window.event)
						{
						case SDL_WINDOWEVENT_RESIZED:
							chip8.whenWindowResized(event.window.data1, event.window.data2);
							continue;
							break;

						case SDL_WINDOWEVENT_CLOSE:
							isRunning = false;
							continue;
							break;
						}
					}
					break;
			}
        }

        if (isPaused || (isSteppingMode && !shouldStep))
        {
        	wasPaused = true;

        	chip8.renderFrameBuffer();

        	chip8.displayDebugInfoIfInDebugMode();

        	if (isPaused)
        	    chip8.setPaused();
        	else
        	    chip8.setDebugTitle();

        	chip8.updateRenderer();

        	// If paused, slow down the cycle to save processing power
        	if (isPaused)
        	    SDL_Delay(frameDelay*500);
        	else
        	    SDL_Delay(frameDelay);

        	// If paused or the stepping key was not pressed, don't execute instruction
        	if (isPaused || !shouldStep)
        	    continue;
        }

        if (wasPaused)
        {
        	chip8.renderFrameBuffer();
        	wasPaused = false;
        }

        chip8.emulateCycle();
        
        // Mark that we executed an instruction since the last step
        shouldStep = false;

        if (chip8.renderFlag)
            chip8.renderFrameBuffer();
        else
            chip8.updateRenderer();
        
        chip8.displayDebugInfoIfInDebugMode();

        SDL_Delay(frameDelay);
    }

    chip8.deinit();
}
