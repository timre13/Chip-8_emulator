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

inline std::string strToLower(const std::string& str)
{
    auto output = str;
    for (auto& c : output)
        c = std::tolower(c);
    return output;
}

class FileChooser final
{
private:
    std::vector<std::string> m_fileList;

    SDL_Window* m_window{};
    SDL_Renderer* m_renderer{};
    TTF_Font* m_font{};

    int m_chosenFileI{};

    void drawFileList() const;
    void drawTitle(const std::string& title) const;
    void drawSelector() const;

public:
    FileChooser(const std::vector<std::string>& directories, const std::vector<std::string>& extensions={"*"});

    std::string get() const;

    ~FileChooser();
};

#endif // SDL_FILE_CHOOSER
