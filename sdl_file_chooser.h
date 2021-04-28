#ifndef SDL_FILE_CHOOSER
#define SDL_FILE_CHOOSER

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define FILECHOOSER_TITLE "Choose a file"

class FileChooser
{
private:
    std::vector<std::string> fileList;

    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    
    int chosenFileI{};

public:
    FileChooser(const std::string &directory, const std::string &extension="*");

    std::string get();

private:
    int getFileList(const std::string &directory, const std::string &extension);
    
    void drawFileList();
    void drawTitle(const std::string &title);
    void drawSelector();

    void deinit();
};

#endif // SDL_FILE_CHOOSER
