#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <filesystem>
#include <algorithm>

extern constexpr double FPS{60};

#include "Chip-8.h"
#include "sdl_file_chooser.h"

//std::vector<std::string> ROMs{};

int main()
{
    /*
    //--- ROM selector ---
    
    for (const auto &entry : std::filesystem::directory_iterator("./roms"))
        ROMs.push_back(entry.path());
    std::sort(ROMs.begin(), ROMs.end());
    
    for (size_t i{}; i < ROMs.size(); ++i)
        std::cout << i << ": " << ROMs[i] << '\n';
    
    size_t romIndex{ROMs.size()-1};
    std::cout << "Enter the number of the ROM to load (default: " <<  romIndex << "): ";
    std::string romIndexStr;
    std::getline(std::cin, romIndexStr);
    std::stringstream ss1{romIndexStr};
    ss1 >> romIndex;
    
    // -------------------
    */
    
    FileChooser fileChooser{"./roms"};
    std::string romFilename{fileChooser.get()};
    
    // If the user canceled the file selection, quit.
    if (romFilename.size() == 0)
        return 0;
    
    // --- speed selector ---

    std::cout << "Emulation speed  (default: 1.0): ";
    double emulationSpeed{1.0};
    std::string emulationSpeedStr{};
    std::getline(std::cin, emulationSpeedStr);
    if (emulationSpeedStr.size())
        emulationSpeed = stod(emulationSpeedStr);
    
    // ----------------------

    Chip8 chip8{romFilename};
    
    std::srand(std::time(nullptr));
    
    std::cout << std::hex;
    
    const double frameDelay{1.0/FPS*100/emulationSpeed};
    
    bool isRunning{true};
    while (isRunning && !chip8.hasEnded)
    {
        SDL_Event event;
    
        SDL_PollEvent(&event);
        
        switch (event.type)
        {
            case SDL_QUIT:
                isRunning = false;
                break;
            
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                }
                break;
        }
        
        chip8.emulateCycle();
        
        if (chip8.renderFlag)
            chip8.renderFrameBuffer();
        else
            chip8.updateRenderer();
        
        SDL_Delay(frameDelay);
        
        //std::string t;
        //std::getline(std::cin, t);
    }
}
