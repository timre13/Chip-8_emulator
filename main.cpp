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
uint8_t bgColorR{175};
uint8_t bgColorG{238};
uint8_t bgColorB{238};

// Color of active pixels
uint8_t fgColorR{70 };
uint8_t fgColorG{130};
uint8_t fgColorb{180};

//------------------------------------------------------------------------------

#include "Chip-8.h"
#include "sdl_file_chooser.h"
#include "DoubleAsker.h"

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
    
    const double frameDelay{1.0/60*100/emulationSpeed};
    
    bool isRunning{true};
    bool isPaused{false};
    bool wasPaused{false};
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
							break;
						case SDLK_F12:
							isRunning = false;
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

        if (isPaused)
        {
        	wasPaused = true;

        	chip8.renderFrameBuffer();

        	chip8.setPaused();

        	chip8.updateRenderer();

        	SDL_Delay(frameDelay*500);

        	continue;
        }

        if (wasPaused)
        {
        	chip8.renderFrameBuffer();
        	wasPaused = false;
        }

        chip8.emulateCycle();
        
        if (chip8.renderFlag)
            chip8.renderFrameBuffer();
        else
            chip8.updateRenderer();
        
        SDL_Delay(frameDelay);
    }

    chip8.deinit();
}
