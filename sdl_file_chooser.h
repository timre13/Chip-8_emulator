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

class FileChooser final
{
private:
    std::vector<std::string> fileList;

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    TTF_Font* font{};
    
    int chosenFileI{};


    int getFileList(const std::string& directory, const std::string& extension);

    void drawFileList() const;
    void drawTitle(const std::string& title) const;
    void drawSelector() const;

    void deinit();

public:
    FileChooser(const std::string& directory, const std::string& extension="*");

    std::string get() const;
};

#endif // SDL_FILE_CHOOSER
