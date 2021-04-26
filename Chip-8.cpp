#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <random>
#include <ctime>
#include <cstring>

#include "Chip-8.h"
#include "fontset.h"
#include "sound.h"
#include "gfx.h"

extern double frameDelay;

//#define ENABLE_DEBUG_TITLE

#define DEBUGGER_TEXTURE_W 300
#define DEBUGGER_TEXTURE_H 420

constexpr uint8_t keyMap[16]{
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_y,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v
    };

constexpr uint8_t keyMapScancode[16]{
    SDL_SCANCODE_X,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_Y,
    SDL_SCANCODE_C,
    SDL_SCANCODE_4,
    SDL_SCANCODE_R,
    SDL_SCANCODE_F,
    SDL_SCANCODE_V
    };

Chip8::Chip8(const std::string &romFilename)
{
    std::srand(std::time(nullptr)); // initialize rand()
    std::rand(); // drop the first result
    
    std::cout << '\n' << "----- setting up video -----" << std::endl;
    Chip8::initVideo();

    // Load the font set to the memory
    Chip8::loadFontSet();

    std::cout << '\n' << "----- loading file -----" << std::endl;
    Chip8::loadFile(romFilename);
}

void Chip8::loadFile(std::string romFilename)
{
    std::cout << "Emulated memory size: " << sizeof(m_memory) / sizeof(m_memory[0]) << std::endl;
    
    std::cout << "Opening file: " << romFilename << std::endl;
    
    FILE *romFile = fopen(romFilename.c_str(), "rb");
    
    if (!romFile)
    {
        std::cerr << "Unable to open file: " << romFilename << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CHIP-8 Emulator", (std::string("Unable to open ROM: ")+romFilename).c_str(), m_window);

        std::exit(2);
    }
    
    fseek(romFile, 0, SEEK_END);
    
    m_romSize = ftell(romFile);
    
    fseek(romFile, 0, SEEK_SET);
    
    std::cout << "File size: " << std::dec << m_romSize << " / 0x" << std::hex << m_romSize << " bytes" << '\n';

    fread(m_memory+512, 8, m_romSize, romFile);

    auto copied = ftell(romFile);
    
    std::cout << "Copied: " << std::dec << copied << std::hex << " bytes" << std::endl;

    if (copied != m_romSize)
    {
        std::cout << "Unable to copy to buffer" << '\n';
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, "Unable to copy file content to memory", m_window);

        std::exit(2);
    }

    std::cout << '\n' << "--- START OF MEMORY ---" << '\n';

    for (int i{}; i < 0xfff+1; ++i)
    {
        std::cout << static_cast<int>(m_memory[i]) << ' ';
        if (i == 0x200-1)
            std::cout << '\n' << "--- START OF PROGRAM ---" << '\n';
        if ((i) == (m_romSize+511))
            std::cout << '\n' << "--- END OF PROGRAM ---" << '\n';
        if (i == 0xfff)
            std::cout << '\n' << "--- END OF MEMORY ---" << '\n';
    }
    std::cout << '\n';
    
    fclose(romFile);
}

void Chip8::loadFontSet()
{
    std::cout << '\n' << "--- FONT SET --- " << '\n';
    for (int i{}; i < 80; ++i)
        std::cout << static_cast<int>(fontset[i]) << ' ';
    std::cout << '\n' << "--- END OF FONT SET ---" << '\n';
    
    // copy the font set to the memory
    for (int i{}; i < 80; ++i)
    {
        m_memory[i] = fontset[i];
    }
}

void Chip8::initVideo()
{
    std::cout << "Initializing SDL" << std::endl;
    
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Unable to initialize SDL. " << SDL_GetError() << '\n';
        std::exit(2);
    }

    std::cout << "Creating window" << std::endl;
    
    m_window = SDL_CreateWindow(
            (std::string(TITLE)+" - Loading...").c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            64*20, 32*20,
            SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    
    if (!m_window)
    {
        std::cerr << "Unable to create window. " << SDL_GetError() << '\n';
        std::exit(2);
    }
    
    std::cout << "Creating renderer" << std::endl;
    
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    
    if (!m_renderer)
    {
        std::cerr << "Unable to create renderer. " << SDL_GetError() << '\n';
        std::exit(2);
    }

    m_contentTexture = SDL_CreateTexture(
            m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
            64, 32);
    if (!m_contentTexture)
    {
        std::cerr << "Unable to create content texture. " << SDL_GetError() << '\n';
        std::exit(2);
    }

    m_debuggerTexture = SDL_CreateTexture(
            m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, DEBUGGER_TEXTURE_W, DEBUGGER_TEXTURE_H);
    if (!m_debuggerTexture)
    {
        std::cerr << "Unable to create debugger texture. " << SDL_GetError() << '\n';
        std::exit(2);
    }
    
    std::cout << "Initializing SDL2_ttf" << std::endl;

    if (TTF_Init())
    {
        std::cerr << "Unable to initialize SDL2_ttf: " << TTF_GetError() << '\n';
        std::exit(2);
    }
    std::cout << "Loading font" << std::endl;

    TTF_Font* font = TTF_OpenFont("Anonymous_Pro.ttf", 16);
    if (!font)
    {
        std::cerr << "Unable to load font: " << TTF_GetError() << '\n';
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, "Unable to load font", m_window);

        std::exit(2);
    }

    std::cout << "Caching font..." << std::endl;
    for (int i{'!'}; i <= '~'; ++i)
    {
        char str[2]{(char)i};

        SDL_Surface* charSurface{TTF_RenderText_Blended(font, (const char*)&str, {255, 255, 255, 255})};
        if (!charSurface)
        {
            std::cerr << "Failed to render font: " << SDL_GetError() << '\n';
            std::exit(2);
        }
        SDL_Texture* charTexture{SDL_CreateTextureFromSurface(m_renderer, charSurface)};
        if (!charTexture)
        {
            std::cerr << "Failed to convert font surface to texture: " << SDL_GetError() << '\n';
            std::exit(2);
        }
        SDL_FreeSurface(charSurface);
        m_fontCache[i - '!'] = charTexture;
    }
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
    SDL_RenderClear(m_renderer);
    updateRenderer();

    SDL_SetWindowMinimumSize(m_window, 64 * 2, 32 * 2);
}

void Chip8::deinit()
{
    if (m_hasDeinitCalled)
        return;
    
    SDL_SetWindowTitle(m_window, (std::string(TITLE)+" - Exiting...").c_str());
    updateRenderer();

    std::cout << '\n' << "----- deinit -----" << std::endl;

    SDL_DestroyTexture(m_contentTexture);
    SDL_DestroyTexture(m_debuggerTexture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    SDL_Quit();

    m_hasDeinitCalled = true;
}

Chip8::~Chip8()
{
    deinit();
}

void Chip8::renderFrameBuffer()
{
    uint8_t* pixelData{};
    int pitch{};
    if (SDL_LockTexture(m_contentTexture, nullptr, (void**)&pixelData, &pitch))
    {
        std::cerr << "Error: Failed to lock content texture: " << SDL_GetError() << std::endl;
        return;
    }

    for (int x{}; x < 64; ++x)
    {
        for (int y{}; y < 32; ++y)
        {
            const int currentPixelI{y*64+x};
            if (m_frameBuffer[currentPixelI])
                Gfx::drawPoint(pixelData, pitch, x, y, SDL_Color{fgColorR, fgColorG, fgColorb});
            else
                Gfx::drawPoint(pixelData, pitch, x, y, SDL_Color{bgColorR, bgColorG, bgColorB});
        }
    }
    SDL_UnlockTexture(m_contentTexture);
    m_renderFlag = false;
}

void Chip8::clearContentTexture()
{
    uint8_t* pixelData{};
    int pitch{};
    if (SDL_LockTexture(m_contentTexture, nullptr, (void**)&pixelData, &pitch))
    {
        std::cerr << "Error: Failed to lock content texture for filling: " << SDL_GetError() << std::endl;
        return;
    }
    Gfx::fillTexture(pixelData, pitch, 32, SDL_Color{bgColorR, bgColorG, bgColorB});
    SDL_UnlockTexture(m_contentTexture);
}

void Chip8::fetchOpcode()
{
    // Catch the access out of the valid memory address range (0x00 - 0xfff)
    assert(m_pc <= 0xfff);
    
    if (m_pc > 0xfff)
    {
        std::cout << "Memory accessed out of range" << std::endl;
        m_hasExited = true;
        return;
    }

    if (m_pc & 1)
    {
        std::cerr << "Tried to fetch opcode from odd address" << std::endl;
        m_hasExited = true;
        return;
    }
    
    // We swap the upper and lower bits.
    // The opcode is 16 bits long, so we have to
    // shift the left part of the opcode and add the right part.
    m_opcode = ((m_memory[m_pc]<<8) | m_memory[m_pc+1]);
    
    std::cout << std::hex;
    std::cout << "PC: 0x" << m_pc << std::endl;
    std::cout << "Current opcode: 0x" << m_opcode << std::endl;
    
    m_pc += 2;
}

void Chip8::setDebugTitle()
{
#ifdef ENABLE_DEBUG_TITLE
    SDL_SetWindowTitle(m_window,
        (std::string(TITLE)+" - "+
            "PC: "      + std::to_string(m_pc)         + " "
            "I: "       + std::to_string(m_indexReg)          + " "
            "SP: "      + std::to_string(m_sp)         + " "
            "DT: "      + std::to_string(m_delayTimer) + " "
            "ST: "      + std::to_string(m_soundTimer) + " "
            "Opcode: "  + std::to_string(m_opcode)     + " "
        ).c_str());
#else
    // If the window title is not the default
    if (std::strcmp(SDL_GetWindowTitle(m_window), TITLE))
        // Set the default title
        SDL_SetWindowTitle(m_window, TITLE);
#endif
}

void Chip8::setPaused()
{
    SDL_SetWindowTitle(m_window, (std::string(TITLE)+" - [PAUSED]").c_str());

    SDL_Rect rect{0,
                  0,
                  static_cast<int>(std::ceil(64*20*m_scale)),
                  static_cast<int>(std::ceil(32*20*m_scale))
    };

    // Make the window darker
    SDL_SetRenderDrawColor(m_renderer, bgColorR, bgColorG, bgColorB, 150);
    SDL_RenderFillRect(m_renderer, &rect);

    updateRenderer();
}

void Chip8::whenWindowResized(int width, int height)
{
    std::cout << "Window resized" << '\n';

    m_windowWidth = width;
    m_windowHeight = height;

    // If the debugger is active, leave space for it in the window
    if (m_isDebugMode)
        width -= DEBUGGER_TEXTURE_W;

    int horizontalScale{width  / 64};
    int verticalScale{height / 32};

    m_scale = std::min(horizontalScale, verticalScale);

    renderFrameBuffer();
}

void Chip8::updateRenderer()
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    { // Game content texture
        SDL_Rect dstRect{0, 0, 64 * m_scale, 32 * m_scale};
        SDL_RenderCopy(m_renderer, m_contentTexture, nullptr, &dstRect);
    }
    if (m_isDebugMode)
    {
        SDL_Rect dstRect{m_windowWidth - DEBUGGER_TEXTURE_W, 0, DEBUGGER_TEXTURE_W, DEBUGGER_TEXTURE_H};
        SDL_RenderCopy(m_renderer, m_debuggerTexture, nullptr, &dstRect);
    }

    SDL_RenderPresent(m_renderer);
}

uint32_t Chip8::getWindowID()
{
    return SDL_GetWindowID(m_window);
}

void Chip8::clearLastRegisterOperationFlags()
{
    m_registers.clearReadWrittenFlags();
}

void Chip8::turnOnFullscreen()
{
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void Chip8::turnOffFullscreen()
{
    SDL_SetWindowFullscreen(m_window, 0);
}

void Chip8::toggleFullscreen()
{
    m_isFullscreen = !m_isFullscreen;

    if (m_isFullscreen)
        turnOnFullscreen();
    else
        turnOffFullscreen();

    // Render the frame buffer with the new scaling
    renderFrameBuffer();
}

void Chip8::toggleCursor()
{
    m_isCursorShown = !m_isCursorShown;

    SDL_ShowCursor(m_isCursorShown);
}

void Chip8::toggleDebugMode()
{
    m_isDebugMode = !m_isDebugMode;

    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    // Call the window resize function to calculate the new
    // scale, so the debug info can fit in the window
    whenWindowResized(w, h);
}

void Chip8::clearIsReadingKeyStateFlag()
{
    m_isReadingKey = false;
}

void Chip8::renderDebugInfoIfInDebugMode()
{
    if (!m_isDebugMode)
        return;

    if (SDL_SetRenderTarget(m_renderer, m_debuggerTexture))
    {
        std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, 255);
    SDL_RenderClear(m_renderer);

    int cursorRow{};
    int cursorCol{};

    auto renderText{[this, &cursorRow, &cursorCol](std::string text, const SDL_Color &color={255, 255, 255, 255}){
        static constexpr int charWidthPx{9};
        static constexpr int charHeightPx{16};

        for (size_t i{}; i < text.length(); ++i)
        {
            const int character{text[i]};
            switch (character)
            {
            case '\n':
                ++cursorRow;
                cursorCol = 0;
                break;

            case '\t':
                cursorCol += 4;
                break;

            case '\v':
                cursorRow += 4;
                cursorCol = 0;
                break;

            case '\r':
                cursorCol = 0;
                break;

            case ' ':
                ++cursorCol;
                break;

            default:
                if (character >= '!' && character <= '~') // If printable character
                {
                    SDL_Rect destRect{
                        cursorCol * charWidthPx + 5, cursorRow * charHeightPx,
                        charWidthPx, charHeightPx};
                    const int textureI{character - '!'};
                    SDL_SetTextureColorMod(m_fontCache[textureI], color.r, color.g, color.b);
                    if (SDL_RenderCopy(m_renderer, m_fontCache[textureI], nullptr, &destRect))
                    {
                        std::cerr << "Failed to copy character texture: " << SDL_GetError() << std::endl;
                    }
                    ++cursorCol;
                }
                else // If unknown nonprintable character
                {
                    ++cursorCol;
                }
                break;
            }
        }
    }};

    renderText("Opcode: " + to_hex(m_opcode) + "\n\n");
    renderText("PC: " + to_hex(m_pc) + "\n\n");
    renderText("I: " + to_hex(m_indexReg) + "\n\n");
    renderText("SP: " + to_hex(m_sp) + "\n\n");

    renderText("Stack:\n");
    for (int i{15}; i >= 0; --i)
        renderText(to_hex(m_stack[i]) + "\n");

    constexpr int indent{17};

    cursorRow = 0;
    cursorCol = indent;
    renderText("Registers:\n");
    for (int i{}; i < 16; ++i)
    {
        cursorCol = indent;

        const bool isRead{m_registers.getIsRegisterRead(i)};
        const bool isWritten{m_registers.getIsRegisterWritten(i)};

        SDL_Color textColor{255, 255, 255, 255};
        if (isRead && isWritten)
            textColor = {255, 255, 0, 255};
        else if (isRead)
            textColor = {0, 255, 0, 255};
        else if (isWritten)
            textColor = {255, 0, 0, 255};

        renderText(to_hex(i, 1) + ": " + to_hex(m_registers.get(i, true)) + "\n", textColor);
    }
    renderText("\n");

    cursorCol = indent;
    renderText("DT: " + to_hex(m_delayTimer) + "\n");
    cursorCol = indent;
    renderText("ST: " + to_hex(m_soundTimer) + "\n\n");

    if (m_isReadingKey)
    {
        cursorCol = indent;
        renderText("Reading keys");
    }

    if (SDL_SetRenderTarget(m_renderer, nullptr))
    {
        std::cerr << "Failed to reset render target: " << SDL_GetError() << std::endl;
    }
}

void Chip8::reportInvalidOpcode(uint8_t opcode)
{
    std::cerr << "Invalid opcode: " << to_hex(opcode) << '\n';

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, TITLE, ("Invalid opcode: "+to_hex(opcode)).c_str(), m_window);
}

void Chip8::emulateCycle()
{
    fetchOpcode();

    setDebugTitle();

    m_timerDecrementCountdown -= frameDelay;

    std::cout << std::hex;

    switch (m_opcode & 0xf000)
    {
        case 0x0000:
            switch (m_opcode & 0x0fff)
            {
                case 0x0000:
                    std::cout << "NOP" << std::endl;
                    break;
                    
                case 0x00e0: // CLS
                    std::cout << "CLS" << std::endl;
                    for (int i{}; i < 64*32; ++i)
                        m_frameBuffer[i] = 0;
                    m_renderFlag = true;
                    break;
                
                case 0x00ee: // RET
                    std::cout << "RET" << std::endl;
                    m_pc = m_stack[m_sp-1];
                    m_stack[m_sp-1] = 0;
                    --m_sp;
                    break;
                    
                default:
                    reportInvalidOpcode(m_opcode);
                    break;
            }
            break;
            
        case 0x1000: // JMP
            std::cout << "JMP" << std::endl;
            m_pc = m_opcode & 0x0fff;
            break;
            
        case 0x2000: // CALL
            std::cout << "CALL" << std::endl;
            ++m_sp;
            m_stack[m_sp-1] = m_pc;
            m_pc = (m_opcode & 0x0fff);
            break;
            
        case 0x3000: // SE
            std::cout << "SE" << std::endl;
            if (m_registers.get((m_opcode & 0x0f00)>>8) == (m_opcode & 0x00ff))
                m_pc += 2;
            break;
            
        case 0x4000: // SNE
            std::cout << "SNE" << std::endl;
            if (m_registers.get((m_opcode & 0x0f00)>>8) != (m_opcode & 0x00ff))
                m_pc += 2;
            break;
        
        case 0x5000: // SE Vx, Vy
            std::cout << "SE Vx, Vy" << std::endl;
            if (m_registers.get((m_opcode & 0x0f00)>>8) == m_registers.get((m_opcode & 0x00f0)>>4))
                m_pc += 2;
            break;
            
        case 0x6000: // LD Vx, byte
            std::cout << "LD Vx, byte" << std::endl;
            m_registers.set((m_opcode & 0x0f00)>>8, m_opcode & 0x00ff);
            break;
        
        case 0x7000: // ADD Vx, byte
            std::cout << "ADD Vx, byte" << std::endl;
            m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) + (m_opcode & 0x00ff));
            break;
        
        case 0x8000:
            switch (m_opcode & 0x000f)
            {
                case 0: // LD Vx, Vy
                    std::cout << "LD Vx, Vy" << std::endl;
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 1: // OR Vx, Vy
                    std::cout << "OR Vx, Vy" << std::endl;
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) | m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 2: // AND Vx, Vy
                    std::cout << "AND Vx, Vy" << std::endl;
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) & m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 3: // XOR Vx, Vy
                    std::cout << "XOR Vx, Vy" << std::endl;
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) ^ m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 4: // ADD Vx, Vy
                    std::cout << "ADD Vx, Vy" << std::endl;
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) + m_registers.get((m_opcode & 0x00f0)>>4));

                    if (m_registers.get((m_opcode & 0x00f0)>>4) > (0xff - m_registers.get((m_opcode & 0x0f00)>>8)))
                        m_registers.set(0xf, 1);
                    else
                        m_registers.set(0xf, 0);
                    break;
                
                case 5: // SUB Vx, Vy
                    std::cout << "SUB Vx, Vy" << std::endl;
                    m_registers.set(0xf, !(m_registers.get((m_opcode & 0x0f00)>>8) < m_registers.get((m_opcode & 0x00f0)>>4)));

                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) - m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 6: // SHR Vx {, Vy}
                    std::cout << "SHR Vx {, Vy}" << std::endl;
                    // Mark whether overflow occurs.
                    m_registers.set(0xf, m_registers.get((m_opcode & 0x0f00)>>8) & 1);

                    if (storeBitShiftResultOfY)
                        m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4) >> 1);
                    else
                        m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) >> 1);
                    break;
                
                case 7: // SUBN Vx, Vy
                    std::cout << "SUBN Vx, Vy" << std::endl;
                    m_registers.set(0xf, !(m_registers.get((m_opcode & 0x0f00)>>8) > m_registers.get((m_opcode & 0x00f0)>>4)));

                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4) - m_registers.get((m_opcode & 0x0f00)>>8));
                    break;
                
                case 0xe: // SHL Vx {, Vy}
                    std::cout << "SDL Vx, {, Vy}" << std::endl;
                    // Mark whether overflow occurs.
                    m_registers.set(0xf, (m_registers.get((m_opcode & 0x0f00)>>8) >> 7));

                    if (storeBitShiftResultOfY)
                        m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4) << 1);
                    else
                        m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) << 1);
                    break;
                
                default:
                    reportInvalidOpcode(m_opcode);
                    break;
            }
            break;
        
        case 0x9000: // SNE Vx, Vy
            std::cout << "SNE Vx, Vy" << std::endl;
            if (m_registers.get((m_opcode & 0x0f00)>>8) !=
                m_registers.get((m_opcode & 0x00f0)>>4))
                m_pc += 2;
            break;
        
        case 0xa000: // LD I, addr
            std::cout << "LD I, addr" << std::endl;
            m_indexReg  = (m_opcode & 0x0fff);
            break;
        
        case 0xb000: // JP V0, addr
            std::cout << "JP V0, addr" << std::endl;
            m_pc = (m_registers.get(0) + (m_opcode & 0x0fff));
            break;
        
        case 0xc000: // RND Vx, byte
            std::cout << "RND Vx, byte" << std::endl;
            m_registers.set((m_opcode & 0x0f00)>>8, (m_opcode & 0x00ff) & static_cast<uint8_t>(std::rand()));
            break;
        
        case 0xd000: // DRW Vx, Vy, nibble
        {
            std::cout << "DRW Vx, Vy, nibble" << std::endl;
            
            int x{m_registers.get((m_opcode & 0x0f00) >> 8)};
            int y{m_registers.get((m_opcode & 0x00f0) >> 4)};
            int height{m_opcode & 0x000f};
            
            m_registers.set(0xf, 0);
            
            for (int cy{}; cy < height; ++cy)
            {
                uint8_t line{m_memory[m_indexReg + cy]};
                
                for (int cx{}; cx < 8; ++cx)
                {
                    uint8_t pixel = line & (0x80 >> cx);
                    
                    if (pixel)
                    {
                        int index{(x + cx) + (y + cy)*64};
                        
                        if (m_frameBuffer[index])
                            m_registers.set(0xf, 1);
                            
                        m_frameBuffer[index] ^= 1;
                    }
                }
            }
            
            m_renderFlag = true;
            
            break;
        }
        
        case 0xe000:
            switch (m_opcode & 0x00ff)
            {
                case 0x9e: // SKP Vx
                {
                    std::cout << "SKP Vx" << std::endl;
                    
                    m_isReadingKey = true;

                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
                    std::cout << "KEY: " << keyState[keyMap[m_registers.get((m_opcode & 0x0f00)>>8)]] << std::endl;
               
                    if (keyState[keyMapScancode[m_registers.get((m_opcode & 0x0f00)>>8)]])
                        m_pc += 2;
                    break;
                }
                
                case 0xa1: // SKNP Vx
                {
                    std::cout << "SKNP Vx" << std::endl;
                    
                    m_isReadingKey = true;

                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
                    std::cout << "KEY: " << keyState[keyMap[m_registers.get((m_opcode & 0x0f00)>>8)]] << std::endl;
               
                    if (!(keyState[keyMapScancode[m_registers.get((m_opcode & 0x0f00)>>8)]]))
                        m_pc += 2;
                    break;
                }
                
                default:
                    reportInvalidOpcode(m_opcode);
                    break;
            }
        break;
        
        case 0xf000:
            switch (m_opcode & 0x00ff)
            {
                case 0x07: // LD Vx, DT
                    std::cout << "LD Vx, DT" << std::endl;
                    m_registers.set((m_opcode & 0x0f00)>>8, m_delayTimer);
                    break;
                
                case 0x0a: // LD Vx, K
                {
                    std::cout << "LD Vx, K" << std::endl;
                    
                    uint16_t pressedKey{};
                    
                    bool hasValidKeyPressed{false};
                    do
                    {
                        setDebugTitle();
                        
                        SDL_SetWindowTitle(m_window,
                            (std::string(SDL_GetWindowTitle(m_window))+
                            std::string(" - waiting for keypress")).c_str());
                        
                        updateRenderer();
                        
                        SDL_Event event;
                        SDL_PollEvent(&event);
                        
                        if (event.type == SDL_KEYDOWN)
                        {
                            for (uint16_t i{}; i < 16; ++i)
                            {
                                if (event.key.keysym.sym == SDLK_F12)
                                {
                                    m_hasExited = true;
                                    hasValidKeyPressed = true;
                                    break;
                                }
                                
                                auto value{keyMap[i]};
                                
                                if (value == event.key.keysym.sym)
                                {
                                    hasValidKeyPressed = true;
                                    pressedKey = i;
                                    break;
                                }
                            }
                        }
                        
                        SDL_Delay(10);
                    }
                    while (!hasValidKeyPressed); // Loop until a valid keypress
                    
                    m_registers.set((m_opcode & 0x0f00)>>8, pressedKey);
                    
                    std::cout << "Loaded key: " << static_cast<int>(pressedKey) << std::endl;
                    
                    break;
                }
                
                case 0x15: // LD DT, Vx
                    std::cout << "LD DT, Vx" << std::endl;
                    m_delayTimer = m_registers.get((m_opcode & 0x0f00)>>8);
                    break;
                
                case 0x18: // LD ST, Vx
                    std::cout << "LD ST, Vx" << std::endl;
                    m_soundTimer = m_registers.get((m_opcode & 0x0f00)>>8);
                    break;
                
                case 0x1e: // ADD I, Vx
                    std::cout << "ADD I, Vx" << std::endl;
                    m_indexReg += m_registers.get((m_opcode & 0x0f00)>>8);
                    break;
                
                case 0x29: // LD F, Vx
                    std::cout << "FD, F, Vx" << std::endl;
                    m_indexReg = m_registers.get((m_opcode & 0x0f00)>>8)*5;
                    std::cout << "FONT LOADED: " << m_registers.get((m_opcode & 0x0f00)>>8) << std::endl;
                    break;
                
                case 0x33: // LD B, Vx
                {
                    std::cout << "LD B, Vx" << std::endl;
                    uint8_t number{m_registers.get((m_opcode & 0x0f00)>>8)};
                    m_memory[m_indexReg] = (number / 100);
                    m_memory[m_indexReg+1] = ((number / 10) % 10);
                    m_memory[m_indexReg+2] = (number % 10);
                    break;
                }
                
                case 0x55: // LD [I], Vx
                {
                    std::cout << "LD [I], Vx" << std::endl;
                    uint8_t x{static_cast<uint8_t>((m_opcode & 0x0f00)>>8)};
                        
                    for (uint8_t i{}; i <= x; ++i)
                        m_memory[m_indexReg + i] = m_registers.get(i);
                    
                    if (incrementIAfterMemoryOperation)
                        m_indexReg += (x + 1);
                    break;
                }
                
                case 0x65: // LD Vx, [I]
                {
                    std::cout << "LD Vx, [I]" << std::endl;
                    uint8_t x{static_cast<uint8_t>((m_opcode & 0x0f00)>>8)};
                    
                    for (uint8_t i{}; i <= x; ++i)
                        m_registers.set(i, m_memory[m_indexReg + i]);
                    
                    if (incrementIAfterMemoryOperation)
                        m_indexReg += (x + 1);
                    break;
                }
                
                default:
                    reportInvalidOpcode(m_opcode);
                    break;
            }
        break;
            
        default:
            reportInvalidOpcode(m_opcode);
            break;
    }
    
    if (m_timerDecrementCountdown <= 0)
    {
        if (m_delayTimer > 0)
            --m_delayTimer;

        if (m_soundTimer > 0)
        {
            --m_soundTimer;
            Sound::makeBeepSound();
        }

        // reset the timer
        m_timerDecrementCountdown = 16.67;
    }
}
