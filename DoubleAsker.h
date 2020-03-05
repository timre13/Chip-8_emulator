#ifndef DOUBLE_ASKER
#define DOUBLE_ASKER

#include <SDL2/SDL.h>
#include "SDL_ttf.h"
#include <string>
#include <iostream>

#define DOUBLEASKER_TITLE "Enter the emulation speed:"

class DoubleAsker
{
private:
    std::string enteredValue;
    
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    
    void drawTitle(const std::string &title);
    void drawEnteredValue();

public:
    DoubleAsker();
    
    double get();
    
    void init();
    void deinit();
};

#endif // DOUBLE_ASKER
