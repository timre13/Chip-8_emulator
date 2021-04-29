#include <string>
#include <fstream>
#include <cassert>
#include <random>
#include <ctime>
#include <cstring>
#include <string>

#include "Chip-8.h"
#include "fontset.h"
#include "sound.h"
#include "gfx.h"
#include "config.h"

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
    
    Logger::log << '\n' << "----- setting up video -----" << Logger::End;
    Chip8::initVideo();

    // Load the font set to the memory
    Chip8::loadFontSet();

    Logger::log << '\n' << "----- loading file -----" << Logger::End;
    Chip8::loadFile(romFilename);
}

void Chip8::loadFile(std::string romFilename)
{
    Logger::log << "Emulated memory size: " << sizeof(m_memory) / sizeof(m_memory[0]) << Logger::End;
    
    Logger::log << "Opening file: " << romFilename << Logger::End;
    
    FILE *romFile = fopen(romFilename.c_str(), "rb");
    
    if (!romFile)
    {
        Logger::err << "Unable to open file: " << romFilename << Logger::End;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CHIP-8 Emulator", (std::string("Unable to open ROM: ")+romFilename).c_str(), m_window);

        std::exit(2);
    }
    
    fseek(romFile, 0, SEEK_END);
    
    m_romSize = ftell(romFile);
    
    fseek(romFile, 0, SEEK_SET);
    
    Logger::log << "File size: " << std::dec << m_romSize << " / 0x" << std::hex << m_romSize << " bytes" << Logger::End;

    fread(m_memory+512, 8, m_romSize, romFile);

    auto copied = ftell(romFile);
    
    Logger::log << "Copied: " << std::dec << copied << std::hex << " bytes" << Logger::End;

    if (copied != m_romSize)
    {
        Logger::err << "Unable to copy to buffer" << Logger::End;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, "Unable to copy file content to memory", m_window);

        std::exit(2);
    }

    Logger::log << '\n' << "--- START OF MEMORY ---" << Logger::End;

    for (int i{}; i < 0xfff+1; ++i)
    {
        Logger::log << static_cast<int>(m_memory[i]) << ' ';
        if (i == 0x200-1)
            Logger::log << '\n' << "--- START OF PROGRAM ---" << '\n';
        if ((i) == (m_romSize+511))
            Logger::log << '\n' << "--- END OF PROGRAM ---" << '\n';
        if (i == 0xfff)
            Logger::log << '\n' << "--- END OF MEMORY ---" << '\n';
    }
    Logger::log << Logger::End;
    
    fclose(romFile);
}

void Chip8::loadFontSet()
{
    Logger::log << '\n' << "--- FONT SET --- " << '\n';
    for (int i{}; i < 80; ++i)
        Logger::log << static_cast<int>(fontset[i]) << ' ';
    Logger::log << '\n' << "--- END OF FONT SET ---" << Logger::End;
    
    // copy the font set to the memory
    for (int i{}; i < 80; ++i)
    {
        m_memory[i] = fontset[i];
    }
}

void Chip8::initVideo()
{
    Logger::log << "Initializing SDL" << Logger::End;
    
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        Logger::err << "Unable to initialize SDL. " << SDL_GetError() << Logger::End;
        std::exit(2);
    }

    Logger::log << "Creating window" << Logger::End;
    
    m_window = SDL_CreateWindow(
            (std::string(TITLE)+" - Loading...").c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            64*20, 32*20,
            SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    
    if (!m_window)
    {
        Logger::err << "Unable to create window. " << SDL_GetError() << Logger::End;
        std::exit(2);
    }
    
    Logger::log << "Creating renderer" << Logger::End;
    
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    
    if (!m_renderer)
    {
        Logger::err << "Unable to create renderer. " << SDL_GetError() << Logger::End;
        std::exit(2);
    }

    m_contentTexture = SDL_CreateTexture(
            m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
            64, 32);
    if (!m_contentTexture)
    {
        Logger::err << "Unable to create content texture. " << SDL_GetError() << Logger::End;
        std::exit(2);
    }

    m_debuggerTexture = SDL_CreateTexture(
            m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, DEBUGGER_TEXTURE_W, DEBUGGER_TEXTURE_H);
    if (!m_debuggerTexture)
    {
        Logger::err << "Unable to create debugger texture. " << SDL_GetError() << Logger::End;
        std::exit(2);
    }
    
    Logger::log << "Initializing SDL2_ttf" << Logger::End;

    if (TTF_Init())
    {
        Logger::err << "Unable to initialize SDL2_ttf: " << TTF_GetError() << Logger::End;
        std::exit(2);
    }
    Logger::log << "Loading font" << Logger::End;

    TTF_Font* font = TTF_OpenFont("Anonymous_Pro.ttf", 16);
    if (!font)
    {
        Logger::err << "Unable to load font: " << TTF_GetError() << Logger::End;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, "Unable to load font", m_window);

        std::exit(2);
    }

    Logger::log << "Caching font..." << Logger::End;
    for (int i{'!'}; i <= '~'; ++i)
    {
        char str[2]{(char)i};

        SDL_Surface* charSurface{TTF_RenderText_Blended(font, (const char*)&str, {255, 255, 255, 255})};
        if (!charSurface)
        {
            Logger::err << "Failed to render font: " << SDL_GetError() << Logger::End;
            std::exit(2);
        }
        SDL_Texture* charTexture{SDL_CreateTextureFromSurface(m_renderer, charSurface)};
        if (!charTexture)
        {
            Logger::err << "Failed to convert font surface to texture: " << SDL_GetError() << Logger::End;
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

    Logger::log << '\n' << "----- deinit -----" << Logger::End;

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
        Logger::err << "Error: Failed to lock content texture: " << SDL_GetError() << Logger::End;
        return;
    }

    for (int x{}; x < 64; ++x)
    {
        for (int y{}; y < 32; ++y)
        {
            const int currentPixelI{y*64+x};
            if (m_frameBuffer[currentPixelI])
                Gfx::drawPoint(pixelData, pitch, x, y, SDL_Color{FG_COLOR_R, FG_COLOR_G, FG_COLOR_B});
            else
                Gfx::drawPoint(pixelData, pitch, x, y, SDL_Color{BG_COLOR_R, BG_COLOR_G, BG_COLOR_B});
        }
    }
    SDL_UnlockTexture(m_contentTexture);
    m_renderFlag = false;
}

void Chip8::fetchOpcode()
{
    // Catch the access out of the valid memory address range (0x00 - 0xfff)
    assert(m_pc <= 0xfff);
    
    if (m_pc > 0xfff)
    {
        Logger::err << "Memory accessed out of range" << Logger::End;
        m_hasExited = true;
        return;
    }
    
    // We swap the upper and lower bits.
    // The opcode is 16 bits long, so we have to
    // shift the left part of the opcode and add the right part.
    m_opcode = ((m_memory[m_pc]<<8) | m_memory[m_pc+1]);
    
#if VERBOSE_LOG
    Logger::log << std::hex;
    Logger::log << "PC: 0x" << m_pc << Logger::End;
    Logger::log << "Current opcode: 0x" << m_opcode << Logger::End;
#endif
    
    m_pc += 2;
}

void Chip8::whenWindowResized(int width, int height)
{
    Logger::log << "Window resized" << Logger::End;

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

    SDL_ShowCursor(false);
}

void Chip8::turnOffFullscreen()
{
    SDL_SetWindowFullscreen(m_window, 0);

    SDL_ShowCursor(true);
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
    // SDL_ShowCursor(-1) returns whether the cursor is shown
    SDL_ShowCursor(!SDL_ShowCursor(-1));
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

std::string Chip8::dumpStateToStr()
{
    std::stringstream output;

    output << "Memory:\n";
    output << std::hex;
    for (int i{}; i < 0x1000; ++i)
    {
        output << std::setw(2) << std::setfill('0') << +m_memory[i] << ' ';
        if (i % 32 == 31)
            output << '\n';
    }

    output << "\nFramebuffer:\n";
    for (int i{}; i < 64 * 32; ++i)
    {
        output << +m_frameBuffer[i] << ' ';
        if (i % 64 == 63)
            output << '\n';
    }

    output << "\nPC=" << std::setw(4) << std::setfill('0') << +m_pc
           << ", Op=" << std::setw(4) << std::setfill('0') << +m_opcode
           << ", SP=" << std::setw(1) << std::setfill('0') << +m_sp
           << ", I="  << std::setw(4) << std::setfill('0') << +m_indexReg
           << ", DT=" << std::setw(2) << std::setfill('0') << +m_delayTimer
           << ", ST=" << std::setw(2) << std::setfill('0') << +m_soundTimer
           << '\n';
    for (int i{}; i < 16; ++i)
    {
        output << i << '=' << std::setw(2) << std::setfill('0') << +m_registers.get(i, true);
        if (i != 15)
            output << ", ";
    }

    output << "\n\nStack:\n";
    for (int i{15}; i > -1; --i)
    {
        output << std::setw(4) << std::setfill('0') << m_stack[i];
        if (i + 1 == m_sp)
            output << " <-"; // Mark the stack pointer
        output << '\n';
    }

    return output.str();
}

void Chip8::renderDebugInfoIfInDebugMode()
{
    if (!m_isDebugMode)
        return;

    if (SDL_SetRenderTarget(m_renderer, m_debuggerTexture))
    {
        Logger::err << "Failed to set render target: " << SDL_GetError() << Logger::End;
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
                        Logger::err << "Failed to copy character texture: " << SDL_GetError() << Logger::End;
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
        Logger::err << "Failed to reset render target: " << SDL_GetError() << Logger::End;
    }
}

void Chip8::updateWindowTitle()
{
    if (m_isPaused)
        SDL_SetWindowTitle(m_window, TITLE " - [PAUSED]");
    else
        SDL_SetWindowTitle(m_window, (TITLE " - Speed: " + std::to_string(m_emulSpeedPerc) + "%").c_str());
}

void Chip8::emulateCycle()
{
    fetchOpcode();

    m_timerDecrementCountdown -= m_frameDelay;

    auto logOpcode{[](const std::string &str){
#if VERBOSE_LOG
        Logger::log << str << Logger::End;
#else
        (void)str;
#endif
    }};

    auto reportInvalidOpcode{[this](uint8_t opcode){
        Logger::err << "Invalid opcode: " << to_hex(opcode) << Logger::End;
        Logger::log << '\n' << dumpStateToStr() << Logger::End;

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, TITLE, "Invalid opcode, see the terminal for more information", m_window);
    }};

    Logger::log << std::hex;

    switch (m_opcode & 0xf000)
    {
        case 0x0000:
            switch (m_opcode & 0x0fff)
            {
                case 0x0000:
                    logOpcode("NOP");
                    break;
                    
                case 0x00e0: // CLS
                    logOpcode("CLS");
                    for (int i{}; i < 64*32; ++i)
                        m_frameBuffer[i] = 0;
                    m_renderFlag = true;
                    break;
                
                case 0x00ee: // RET
                    logOpcode("RET");
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
            logOpcode("JMP");
            m_pc = m_opcode & 0x0fff;
            break;
            
        case 0x2000: // CALL
            logOpcode("CALL");
            ++m_sp;
            m_stack[m_sp-1] = m_pc;
            m_pc = (m_opcode & 0x0fff);
            break;
            
        case 0x3000: // SE
            logOpcode("SE");
            if (m_registers.get((m_opcode & 0x0f00)>>8) == (m_opcode & 0x00ff))
                m_pc += 2;
            break;
            
        case 0x4000: // SNE
            logOpcode("SNE");
            if (m_registers.get((m_opcode & 0x0f00)>>8) != (m_opcode & 0x00ff))
                m_pc += 2;
            break;
        
        case 0x5000: // SE Vx, Vy
            logOpcode("SE Vx, Vy");
            if (m_registers.get((m_opcode & 0x0f00)>>8) == m_registers.get((m_opcode & 0x00f0)>>4))
                m_pc += 2;
            break;
            
        case 0x6000: // LD Vx, byte
            logOpcode("LD Vx, byte");
            m_registers.set((m_opcode & 0x0f00)>>8, m_opcode & 0x00ff);
            break;
        
        case 0x7000: // ADD Vx, byte
            logOpcode("ADD Vx, byte");
            m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) + (m_opcode & 0x00ff));
            break;
        
        case 0x8000:
            switch (m_opcode & 0x000f)
            {
                case 0: // LD Vx, Vy
                    logOpcode("LD Vx, Vy");
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 1: // OR Vx, Vy
                    logOpcode("OR Vx, Vy");
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) | m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 2: // AND Vx, Vy
                    logOpcode("AND Vx, Vy");
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) & m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 3: // XOR Vx, Vy
                    logOpcode("XOR Vx, Vy");
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) ^ m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 4: // ADD Vx, Vy
                    logOpcode("ADD Vx, Vy");
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) + m_registers.get((m_opcode & 0x00f0)>>4));

                    if (m_registers.get((m_opcode & 0x00f0)>>4) > (0xff - m_registers.get((m_opcode & 0x0f00)>>8)))
                        m_registers.set(0xf, 1);
                    else
                        m_registers.set(0xf, 0);
                    break;
                
                case 5: // SUB Vx, Vy
                    logOpcode("SUB Vx, Vy");
                    m_registers.set(0xf, !(m_registers.get((m_opcode & 0x0f00)>>8) < m_registers.get((m_opcode & 0x00f0)>>4)));

                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) - m_registers.get((m_opcode & 0x00f0)>>4));
                    break;
                
                case 6: // SHR Vx {, Vy}
                    logOpcode("SHR Vx {, Vy}");
                    // Mark whether overflow occurs.
                    m_registers.set(0xf, m_registers.get((m_opcode & 0x0f00)>>8) & 1);

#if SHIFT_Y_REG_INSTEAD_OF_X
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4) >> 1);
#else
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) >> 1);
#endif
                    break;
                
                case 7: // SUBN Vx, Vy
                    logOpcode("SUBN Vx, Vy");
                    m_registers.set(0xf, !(m_registers.get((m_opcode & 0x0f00)>>8) > m_registers.get((m_opcode & 0x00f0)>>4)));

                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4) - m_registers.get((m_opcode & 0x0f00)>>8));
                    break;
                
                case 0xe: // SHL Vx {, Vy}
                    logOpcode("SDL Vx, {, Vy}");
                    // Mark whether overflow occurs.
                    m_registers.set(0xf, (m_registers.get((m_opcode & 0x0f00)>>8) >> 7));

#if SHIFT_Y_REG_INSTEAD_OF_X
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x00f0)>>4) << 1);
#else
                    m_registers.set((m_opcode & 0x0f00)>>8, m_registers.get((m_opcode & 0x0f00)>>8) << 1);
#endif
                    break;
                
                default:
                    reportInvalidOpcode(m_opcode);
                    break;
            }
            break;
        
        case 0x9000: // SNE Vx, Vy
            logOpcode("SNE Vx, Vy");
            if (m_registers.get((m_opcode & 0x0f00)>>8) !=
                m_registers.get((m_opcode & 0x00f0)>>4))
                m_pc += 2;
            break;
        
        case 0xa000: // LD I, addr
            logOpcode("LD I, addr");
            m_indexReg  = (m_opcode & 0x0fff);
            break;
        
        case 0xb000: // JP V0, addr
            logOpcode("JP V0, addr");
            m_pc = (m_registers.get(0) + (m_opcode & 0x0fff));
            break;
        
        case 0xc000: // RND Vx, byte
            logOpcode("RND Vx, byte");
            m_registers.set((m_opcode & 0x0f00)>>8, (m_opcode & 0x00ff) & static_cast<uint8_t>(std::rand()));
            break;
        
        case 0xd000: // DRW Vx, Vy, nibble
        {
            logOpcode("DRW Vx, Vy, nibble");
            
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
                    logOpcode("SKP Vx");
                    
                    m_isReadingKey = true;

                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
#if VERBOSE_LOG
                    Logger::log << "KEY: " << keyState[keyMap[m_registers.get((m_opcode & 0x0f00)>>8)]] << Logger::End;
#endif
               
                    if (keyState[keyMapScancode[m_registers.get((m_opcode & 0x0f00)>>8)]])
                        m_pc += 2;
                    break;
                }
                
                case 0xa1: // SKNP Vx
                {
                    logOpcode("SKNP Vx");
                    
                    m_isReadingKey = true;

                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
#if VERBOSE_LOG
                    Logger::log << "KEY: " << keyState[keyMap[m_registers.get((m_opcode & 0x0f00)>>8)]] << Logger::End;
#endif
               
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
                    logOpcode("LD Vx, DT");
                    m_registers.set((m_opcode & 0x0f00)>>8, m_delayTimer);
                    break;
                
                case 0x0a: // LD Vx, K
                {
                    logOpcode("LD Vx, K");
                    
                    SDL_SetWindowTitle(m_window, TITLE " - waiting for keypress");
                    
                    uint16_t pressedKey{};
                    bool hasValidKeyPressed{false};
                    do
                    {
                        updateRenderer();
                        
                        SDL_Event event;
                        SDL_PollEvent(&event);
                        
                        if (event.type == SDL_KEYDOWN)
                        {
                            for (uint16_t i{}; i < 16; ++i)
                            {
                                if (event.key.keysym.sym == SHORTCUT_KEYCODE_QUIT)
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

                    // Reset the title
                    updateWindowTitle();
                    
                    m_registers.set((m_opcode & 0x0f00)>>8, pressedKey);
                    
#if VERBOSE_LOG
                    Logger::log << "Loaded key: " << static_cast<int>(pressedKey) << Logger::End;
#endif
                    
                    break;
                }
                
                case 0x15: // LD DT, Vx
                    logOpcode("LD DT, Vx");
                    m_delayTimer = m_registers.get((m_opcode & 0x0f00)>>8);
                    break;
                
                case 0x18: // LD ST, Vx
                    logOpcode("LD ST, Vx");
                    m_soundTimer = m_registers.get((m_opcode & 0x0f00)>>8);
                    break;
                
                case 0x1e: // ADD I, Vx
                    logOpcode("ADD I, Vx");
                    m_indexReg += m_registers.get((m_opcode & 0x0f00)>>8);
                    break;
                
                case 0x29: // LD F, Vx
                    logOpcode("FD, F, Vx");
                    m_indexReg = m_registers.get((m_opcode & 0x0f00)>>8)*5;
#if VERBOSE_LOG
                    Logger::log << "FONT LOADED: " << m_registers.get((m_opcode & 0x0f00)>>8) << Logger::End;
#endif
                    break;
                
                case 0x33: // LD B, Vx
                {
                    logOpcode("LD B, Vx");
                    uint8_t number{m_registers.get((m_opcode & 0x0f00)>>8)};
                    m_memory[m_indexReg] = (number / 100);
                    m_memory[m_indexReg+1] = ((number / 10) % 10);
                    m_memory[m_indexReg+2] = (number % 10);
                    break;
                }
                
                case 0x55: // LD [I], Vx
                {
                    logOpcode("LD [I], Vx");
                    uint8_t x{static_cast<uint8_t>((m_opcode & 0x0f00)>>8)};
                        
                    for (uint8_t i{}; i <= x; ++i)
                        m_memory[m_indexReg + i] = m_registers.get(i);
                    
#if INC_I_AFTER_MEM_OP
                    m_indexReg += (x + 1);
#endif
                    break;
                }
                
                case 0x65: // LD Vx, [I]
                {
                    logOpcode("LD Vx, [I]");
                    uint8_t x{static_cast<uint8_t>((m_opcode & 0x0f00)>>8)};
                    
                    for (uint8_t i{}; i <= x; ++i)
                        m_registers.set(i, m_memory[m_indexReg + i]);
                    
#if INC_I_AFTER_MEM_OP
                    m_indexReg += (x + 1);
#endif
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
