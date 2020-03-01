/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "sdl_file_chooser.h"

void FileChooser::getFileList(std::string directory)
{
    auto files{std::filesystem::directory_iterator{directory}};
    
    for (auto &file : files)
        fileList.push_back(file.path().c_str());
    
    std::sort(fileList.begin(), fileList.end());
}

void FileChooser::drawTitle()
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, FILECHOOSER_TITLE, {255, 255, 255, 255});
    
    SDL_Rect sourceRect{0, 0, textSurface->w,        textSurface->h};
    SDL_Rect targetRect{10, 10, textSurface->w / 3,    textSurface->h / 3};
    
    SDL_Texture* textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};
    
    SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void FileChooser::drawFileList()
{
    for (int i{}; i < static_cast<int>(fileList.size()); ++i)
    {
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, fileList[i].c_str(), {255, 255, 255, 255});
        
        SDL_Rect sourceRect{0, 0, textSurface->w,        textSurface->h};
        SDL_Rect targetRect{0, 50+i*30, textSurface->w / 5,    textSurface->h / 5};
        
        SDL_Texture* textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};
        
        SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);
        
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
    }
}

FileChooser::FileChooser(std::string directory)
{
    getFileList(directory);
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 1000, SDL_WINDOW_ALLOW_HIGHDPI);
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    
    font = TTF_OpenFont("./Anonymous_Pro.ttf", 100);
    
    if (!font)
    {
        std::cerr << "Unable to open font file." << '\n';
        std::exit(2);
    }

    bool isRunning{true};
    while (isRunning)
    {
        SDL_Event event;
        
        SDL_PollEvent(&event);
        
        switch (event.type)
        {
            case SDL_QUIT:
                chosenFileI = -1;
                isRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        chosenFileI = -1;
                        isRunning = false;
                        break;
                    
                    case SDLK_DOWN:
                        chosenFileI += 0.4;
                        
                        if (chosenFileI > fileList.size()-1)
                            chosenFileI = fileList.size()-1;
                        break;
                    
                    case SDLK_UP:
                        chosenFileI -= 0.4;
                        
                        if (chosenFileI < 0)
                            chosenFileI = 0;
                        break;
                    
                    case SDLK_RETURN:
                        isRunning = false;
                        deinit();
                        return;
                }
            break;
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
        SDL_Rect selectorRect{0, 50+static_cast<int>(chosenFileI)*30, 800, 25};
        SDL_RenderFillRect(renderer, &selectorRect);
        
        drawTitle();
        drawFileList();
        
        SDL_RenderPresent(renderer);
        
        SDL_Delay(20);
    }
    
    deinit();
}

std::string FileChooser::get()
{
    if (chosenFileI != -1)
        return fileList.at(chosenFileI);
    
    return ""; // return an empty string if the user closed the window
}

void FileChooser::deinit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    TTF_CloseFont(font);
    
    TTF_Quit();
    SDL_Quit();
}
