#include "DoubleAsker.h"
#include <sstream>

DoubleAsker::DoubleAsker()
{
    init();
}

void DoubleAsker::init()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 100, SDL_WINDOW_ALLOW_HIGHDPI);
    
    if (!window)
    {
        std::cerr << "Unable to create window" << '\n';
        std::exit(2);
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    
    if (!renderer)
    {
        std::cerr << "Unable to create renderer" << '\n';
        std::exit(2);
    }
    
    font = TTF_OpenFont("./Anonymous_Pro.ttf", 100);
    
    if (!font)
    {
        std::cerr << "Unable to open font file." << '\n';
        std::exit(2);
    }
    
    SDL_Delay(100);

    bool isRunning{true};
    while (isRunning)
    {
        SDL_Event event;
        
        SDL_PollEvent(&event);
        
        switch (event.type)
        {
            case SDL_QUIT:
                enteredValue = -1;
                isRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        enteredValue = -1;
                        isRunning = false;
                        break;
                    
                    case SDLK_RETURN:
                        isRunning = false;
                        return;
            
                    default:
                        char pressedKey{static_cast<char>(event.key.keysym.sym)};
                        int pressedKeyInt{static_cast<int>(pressedKey) - 48};
                        
                        // If the pressed key is a digit or a point
                        if (
                                ((pressedKeyInt < 10) &&
                                (pressedKeyInt >= 0)) ||
                                (pressedKeyInt == -2))
                        {
                            enteredValue += pressedKey;
                        }
                        break;
                }
            break;
        }
        
        if (!isRunning)
        	break;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawTitle(DOUBLEASKER_TITLE);
        drawEnteredValue();
        
        SDL_RenderPresent(renderer);
        
        SDL_Delay(20);
    }
    
    deinit();
}

void DoubleAsker::drawTitle(const std::string &title)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, title.c_str(), {255, 255, 255, 255});
    
    SDL_Rect sourceRect{0, 0, textSurface->w,        textSurface->h};
    SDL_Rect targetRect{10, 10, textSurface->w / 3,    textSurface->h / 3};
    
    SDL_Texture* textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};
    
    SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void DoubleAsker::drawEnteredValue()
{
    if (enteredValue.size() == 0)
        return;
    
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, enteredValue.c_str(), {255, 255, 255, 255});
    
    SDL_Rect sourceRect{0, 0, textSurface->w,        textSurface->h};
    SDL_Rect targetRect{10, 50, textSurface->w / 4,    textSurface->h / 4};
    
    SDL_Texture* textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};
    
    SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

double DoubleAsker::get()
{
    std::stringstream ss{enteredValue};
    
    double value;
    
    ss >> value;
    
    // If the user left the default value
    if (enteredValue.size() == 0)
        value = 1;
    
    deinit();

    return value;
}

void DoubleAsker::deinit()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    
    SDL_Quit();
    TTF_Quit();
}
