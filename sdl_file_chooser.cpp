#include "sdl_file_chooser.h"
#include "submodules/chip8asm/src/Logger.h"
#include <cmath>
#include <stdint.h>
#include <string>

namespace std_fs = std::filesystem;

static void getFileList(
        const std::vector<std::string>& dirs, const std::vector<std::string>& exts,
        std::vector<std::string>* output)
{
    std::vector<std::string> filePaths;
    for (const auto& dir : dirs)
    {
        try
        {
            std_fs::recursive_directory_iterator iterator = {dir, std_fs::directory_options::skip_permission_denied};
            for (const auto& filePath : iterator)
            {
                filePaths.push_back(filePath.path());
            }
        }
        catch (const std::exception& e)
        {
            Logger::err << "Failed to list directory: " << dir << ": " << e.what() << Logger::End;
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "File Chooser Error",
                    ("Failed to list directory: "+dir+": "+e.what()).c_str(), nullptr);
        }
    }

    const bool shouldFilter = std::find(exts.begin(), exts.end(), "*") == exts.end();
    for (const auto& path : filePaths)
    {
        try
        {
            if (!std_fs::is_regular_file(path))
            {
                continue;
            }
        }
        catch (...)
        {
            continue;
        }

        if (shouldFilter)
        {
            std::string fileExt = strToLower(std_fs::path(path).extension().string());
            if (!fileExt.empty()) fileExt = fileExt.substr(1);
            if (std::find(exts.begin(), exts.end(), fileExt) == exts.end())
                continue;
        }

        // Don't write if the list already contains the value (filter duplicates)
        if (std::find(output->begin(), output->end(), path) != output->end())
            continue;

        output->push_back(path);
    }
    
    std::sort(output->begin(), output->end());
}

void FileChooser::drawTitle(const std::string& title) const
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(m_font, (m_fileList.size() ? title : "Empty file list").c_str(), {255, 255, 255, 255});
    
    SDL_Rect sourceRect{0, 0, textSurface->w, textSurface->h};
    SDL_Rect targetRect{10, 10, textSurface->w / 3, textSurface->h / 3};
    
    SDL_Texture* textTexture{SDL_CreateTextureFromSurface(m_renderer, textSurface)};
    
    SDL_RenderCopy(m_renderer, textTexture, &sourceRect, &targetRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void FileChooser::drawFileList() const
{
    for (int i{}; i < static_cast<int>(m_fileList.size()); ++i)
    {
        int y{500 - m_chosenFileI * 30 + i * 30};
        
        if (y  < 1000 && y > 0)
        {
            SDL_Surface *textSurface = TTF_RenderText_Blended(
                m_font,
                m_fileList[i].c_str(),
                {255, 255, 255, static_cast<uint8_t>(255 - abs(500 - y) / 2)});
            
            SDL_Rect sourceRect{0, 0, textSurface->w, textSurface->h};
            SDL_Rect targetRect{0, y, textSurface->w / 5, textSurface->h / 5};
            
            SDL_Texture* textTexture{SDL_CreateTextureFromSurface(m_renderer, textSurface)};
            
            SDL_RenderCopy(m_renderer, textTexture, &sourceRect, &targetRect);
            
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
}

void FileChooser::drawSelector() const
{
    SDL_SetRenderDrawColor(m_renderer, 100, 100, 100, 100);
    SDL_Rect selectorRect{0, 500, 800, 25};
    SDL_RenderFillRect(m_renderer, &selectorRect);
}

FileChooser::FileChooser(const std::vector<std::string>& directories, const std::vector<std::string>& extensions/*={"*"}*/)
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    m_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 1000, SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window)
    {
        Logger::err << "Unable to create window" << Logger::End;
        std::exit(2);
    }
    
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer)
    {
        Logger::err << "Unable to create renderer" << Logger::End;
        std::exit(2);
    }
    
    m_font = TTF_OpenFont("./Anonymous_Pro.ttf", 100);
    if (!m_font)
    {
        Logger::err << "Unable to open font file." << Logger::End;
        std::exit(2);
    }

    drawTitle("Loading...");
    SDL_RenderPresent(m_renderer);
    getFileList(directories, extensions, &m_fileList);

    bool isRunning{true};
    while (isRunning)
    {
        SDL_Event event;
        
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                m_chosenFileI = -1;
                isRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        m_chosenFileI = -1;
                        isRunning = false;
                        break;

                    case SDLK_DOWN:
                    case SDLK_j:
                        m_chosenFileI += 1;

                        if (m_chosenFileI > static_cast<int>(m_fileList.size())-1)
                            m_chosenFileI = m_fileList.size()-1;
                        break;

                    case SDLK_UP:
                    case SDLK_k:
                        m_chosenFileI -= 1;

                        if (m_chosenFileI < 0)
                            m_chosenFileI = 0;
                        break;

                    case SDLK_RETURN:
                        isRunning = false;
                        return;
                }
                break;

            case SDL_MOUSEWHEEL:
                if (event.wheel.y < 0)
                {
                    m_chosenFileI += 1;

                    if (m_chosenFileI > static_cast<int>(m_fileList.size())-1)
                        m_chosenFileI = m_fileList.size()-1;
                }
                else if (event.wheel.y > 0)
                {
                    m_chosenFileI -= 1;

                    if (m_chosenFileI < 0)
                        m_chosenFileI = 0;
                }
                break;
            }
        }
        
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);

        drawSelector();
        drawTitle(FILECHOOSER_TITLE);
        drawFileList();
        
        SDL_RenderPresent(m_renderer);
        
        SDL_Delay(20);
    }
}

std::string FileChooser::get() const
{
    // Return an empty string if there are no files to choose from
    // or the user closed the window
    if (m_fileList.size() == 0 || m_chosenFileI == -1)
        return "";

    return m_fileList.at(m_chosenFileI);
}

FileChooser::~FileChooser()
{
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    TTF_CloseFont(m_font);
    
    TTF_Quit();
    SDL_Quit();
}
