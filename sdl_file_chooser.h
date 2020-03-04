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

#ifndef SDL_FILE_CHOOSER
#define SDL_FILE_CHOOSER

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#include "SDL_ttf.h"

#define FILECHOOSER_TITLE "Choose a file"

class FileChooser
{
private:
    std::vector<std::string> fileList;

    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    
    int chosenFileI{};

    void getFileList(std::string directory);
    
    void drawFileList();
    void drawTitle(const std::string &title);
    void drawSelector();

    void deinit();

public:
    FileChooser(std::string directory);
    
    std::string get();
};

#endif // SDL_FILE_CHOOSER
