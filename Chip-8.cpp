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

extern double frameDelay;

//#define ENABLE_DEBUG_TITLE

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
    std::cout << "Emulated memory size: " << sizeof(memory) / sizeof(memory[0]) << std::endl;
    
    std::cout << "Opening file: " << romFilename << std::endl;
    
    FILE *romFile = fopen(romFilename.c_str(), "rb");
    
    if (!romFile)
    {
        std::cerr << "Unable to open file: " << romFilename << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CHIP-8 Emulator", (std::string("Unable to open ROM: ")+romFilename).c_str(), window);

        std::exit(2);
    }
    
    fseek(romFile, 0, SEEK_END);
    
    romSize = ftell(romFile);
    
    fseek(romFile, 0, SEEK_SET);
    
    std::cout << "File size: " << std::dec << romSize << " / 0x" << std::hex << romSize << " bytes" << '\n';

    fread(memory+512, 8, romSize, romFile);

    auto copied = ftell(romFile);
    
    std::cout << "Copied: " << std::dec << copied << std::hex << " bytes" << std::endl;

    if (copied != romSize)
    {
        std::cout << "Unable to copy to buffer" << '\n';
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, "Unable to copy file content to memory", window);

        std::exit(2);
    }

    std::cout << '\n' << "--- START OF MEMORY ---" << '\n';

    for (int i{}; i < 0xfff+1; ++i)
    {
        std::cout << static_cast<int>(memory[i]) << ' ';
        if (i == 0x200-1)
            std::cout << '\n' << "--- START OF PROGRAM ---" << '\n';
        if ((i) == (romSize+511))
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
        memory[i] = fontset[i];
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
    
    std::cout << "Initializing SDL2_ttf" << std::endl;

    if (TTF_Init())
    {
        std::cerr << "Unable to initialize SDL2_ttf: " << TTF_GetError() << '\n';
        std::exit(2);
    }

    std::cout << "Creating window" << std::endl;
    
    window = SDL_CreateWindow(
            (std::string(TITLE)+" - Loading...").c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            64*20, 32*20,
            SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    
    if (!window)
    {
        std::cerr << "Unable to create window. " << SDL_GetError() << '\n';
        std::exit(2);
    }
    
    std::cout << "Creating renderer" << std::endl;
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer)
    {
        std::cerr << "Unable to create renderer. " << SDL_GetError() << '\n';
        std::exit(2);
    }
    
    std::cout << "Loading font" << std::endl;

    font = TTF_OpenFont("Anonymous_Pro.ttf", 16);

    if (!font)
    {
        std::cerr << "Unable to load font: " << TTF_GetError() << '\n';
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, "Unable to load font", window);

        std::exit(2);
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    updateRenderer();

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetWindowMinimumSize(window, 200, 100);
}

void Chip8::deinit()
{
    if (hasDeinitCalled)
        return;
    
    SDL_SetWindowTitle(window, (std::string(TITLE)+" - Exiting...").c_str());
    updateRenderer();

    std::cout << '\n' << "----- deinit -----" << std::endl;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(font);

    TTF_Quit();
    SDL_Quit();

    hasDeinitCalled = true;
}

Chip8::~Chip8()
{
    deinit();
}

void Chip8::renderFrameBuffer()
{
    clearRenderer();

    for (int y{}; y < 32; ++y)
    for (int x{}; x < 64; ++x)
    {
        int currentPixelI{y*64+x};
        if (frameBuffer[currentPixelI])
        {
            SDL_SetRenderDrawColor(renderer, fgColorR, fgColorG, fgColorb, 255);
            
            SDL_Rect rect{static_cast<int>(std::ceil(x*20*scale)),
                          static_cast<int>(std::ceil(y*20*scale)),
                          static_cast<int>(std::ceil(20*scale)),
                          static_cast<int>(std::ceil(20*scale))};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    updateRenderer();
    
    renderFlag = false;
}

void Chip8::clearRenderer()
{
    // Only clear the game output, so the debug info can be erased independently

    SDL_SetRenderDrawColor(renderer, bgColorR, bgColorG, bgColorB, 255);
    SDL_Rect rect{0, 0, 64*20*scale, 32*20*scale};
    SDL_RenderFillRect(renderer, &rect);
}

void Chip8::fetchOpcode()
{
    // Catch the access out of the valid memory address range (0x00 - 0xfff)
    assert(pc <= 0xfff);
    
    if (pc > 0xfff)
    {
        std::cout << "Memory accessed out of range" << std::endl;
        hasEnded = true;
        return;
    }
    
    // We swap the upper and lower bits.
    // The opcode is 16 bits long, so we have to
    // shift the left part of the opcode and add the right part.
    opcode = ((memory[pc]<<8) | memory[pc+1]);
    
    std::cout << std::hex;
    std::cout << "PC: 0x" << pc << std::endl;
    std::cout << "Current opcode: 0x" << opcode << std::endl;
    
    pc += 2;
}

void Chip8::setDebugTitle()
{
#ifdef ENABLE_DEBUG_TITLE
    SDL_SetWindowTitle(window,
        (std::string(TITLE)+" - "+
            "PC: "      + std::to_string(pc)         + " "
            "I: "       + std::to_string(I)          + " "
            "SP: "      + std::to_string(sp)         + " "
            "DT: "      + std::to_string(delayTimer) + " "
            "ST: "      + std::to_string(soundTimer) + " "
            "Opcode: "  + std::to_string(opcode)     + " "
        ).c_str());
#else
    // If the window title is not the default
    if (std::strcmp(SDL_GetWindowTitle(window), TITLE))
        // Set the default title
        SDL_SetWindowTitle(window, TITLE);
#endif
}

void Chip8::setPaused()
{
	SDL_SetWindowTitle(window, (std::string(TITLE)+" - [PAUSED]").c_str());

	SDL_Rect rect{0,
				  0,
				  static_cast<int>(std::ceil(64*20*scale)),
				  static_cast<int>(std::ceil(32*20*scale))
	};

	// Make the window darker
	SDL_SetRenderDrawColor(renderer, bgColorR, bgColorG, bgColorB, 150);
	SDL_RenderFillRect(renderer, &rect);

	updateRenderer();
}

void Chip8::whenWindowResized(int width, int height)
{
	std::cout << "Window resized" << '\n';

	double horizontalScale{width  / (64*20.0)};
	double   verticalScale{height / (32*20.0)};

	scale = std::min(horizontalScale, verticalScale);

	if (isDebugMode)
	    scale *= 0.6; // This way the debug info can fit in the window

	SDL_SetRenderDrawColor(renderer, bgColorR, bgColorG, bgColorB, 255);
	SDL_RenderClear(renderer);
	renderFrameBuffer();
}

uint32_t Chip8::getWindowID()
{
	return SDL_GetWindowID(window);
}

void Chip8::clearLastRegisterOperationFlags()
{
    registers.clearReadWrittenFlags();
}

void Chip8::turnOnFullscreen()
{
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void Chip8::turnOffFullscreen()
{
    SDL_SetWindowFullscreen(window, 0);
}

void Chip8::toggleFullscreen()
{
    isFullscreen = !isFullscreen;

    if (isFullscreen)
        turnOnFullscreen();
    else
        turnOffFullscreen();

    // Render the frame buffer with the new scaling
    renderFrameBuffer();
}

void Chip8::toggleCursor()
{
    isCursorShown = !isCursorShown;

    SDL_ShowCursor(isCursorShown);
}

void Chip8::toggleDebugMode()
{
    isDebugMode = !isDebugMode;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    // Call the window resize function to calculate the new
    // scale, so the debug info can fit in the window
    whenWindowResized(w, h);
}

void Chip8::renderText(const std::string &text, int line, int row, const SDL_Color &bgColor)
{
    SDL_Surface *textSurface{TTF_RenderText_Shaded(font, text.c_str(), {255, 255, 255, 255}, bgColor)};
    SDL_Texture *textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};

    SDL_Rect destRect{static_cast<int>(64*20*scale+9*row)+5, 25*line, static_cast<int>(text.length())*9, 25};

    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Chip8::clearDebugInfo()
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect rect{64*20*scale, 0, w-64*20*scale, h};
    SDL_SetRenderDrawColor(renderer, 100, 150, 150, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Chip8::clearIsReadingKeyStateFlag()
{
    isReadingKey = false;
}

void Chip8::displayDebugInfoIfInDebugMode()
{
    if (!isDebugMode)
        return;

    clearDebugInfo();

    renderText("Opcode: "   + to_hex(opcode),       0);
    renderText("PC: "       + to_hex(pc),           2);
    renderText("I: "        + to_hex(I),            4);
    renderText("SP: "       + to_hex(sp),           6);
    renderText("Stack: ",                           8);

    for (int i{15}; i >= 0; --i)
        renderText(to_hex(stack[i]), 9+i);

    renderText("Registers: ",                               0, 20);

    for (int i{}; i < 16; ++i)
    {
        if (registers.getIsRegisterRead(i))
            renderText(to_hex(i, 1) + ": " + to_hex(registers.get(i, true)), 1+i%8, 20+i/8*12, {255, 0, 0, 255});
        else if (registers.getIsRegisterWritten(i))
            renderText(to_hex(i, 1) + ": " + to_hex(registers.get(i, true)), 1+i%8, 20+i/8*12, {0, 255, 0, 255});
        else
            renderText(to_hex(i, 1) + ": " + to_hex(registers.get(i, true)), 1+i%8, 20+i/8*12);
    }

    renderText("DT: "       + to_hex(delayTimer),   10, 20);
    renderText("ST: "       + to_hex(soundTimer),   12, 20);

    if (isReadingKey)
    {
        renderText("Reading key state", 14, 20);
    }

    updateRenderer();
}

void Chip8::reportInvalidOpcode(uint8_t opcode)
{
    std::cerr << "Invalid opcode: " << to_hex(opcode) << '\n';

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, TITLE, ("Invalid opcode: "+to_hex(opcode)).c_str(), window);
}

void Chip8::emulateCycle()
{
    fetchOpcode();

    setDebugTitle();

    timerDecrementCountdown -= frameDelay;

    std::cout << std::hex;

    switch (opcode & 0xf000)
    {
        case 0x0000:
            switch (opcode & 0x0fff)
            {
                case 0x0000:
                    std::cout << "NOP" << std::endl;
                    break;
                    
                case 0x00e0: // CLS
                    std::cout << "CLS" << std::endl;
                    for (int i{}; i < 64*32; ++i)
                        frameBuffer[i] = 0;
                    renderFlag = true;
                    break;
                
                case 0x00ee: // RET
                    std::cout << "RET" << std::endl;
                    pc = stack[sp-1];
                    stack[sp-1] = 0;
                    --sp;
                    break;
                    
                default:
                    reportInvalidOpcode(opcode);
                    break;
            }
            break;
            
        case 0x1000: // JMP
            std::cout << "JMP" << std::endl;
            pc = opcode & 0x0fff;
            break;
            
        case 0x2000: // CALL
            std::cout << "CALL" << std::endl;
            ++sp;
            stack[sp-1] = pc;
            pc = (opcode & 0x0fff);
            break;
            
        case 0x3000: // SE
            std::cout << "SE" << std::endl;
            if (registers.get((opcode & 0x0f00)>>8) == (opcode & 0x00ff))
                pc += 2;
            break;
            
        case 0x4000: // SNE
            std::cout << "SNE" << std::endl;
            if (registers.get((opcode & 0x0f00)>>8) != (opcode & 0x00ff))
                pc += 2;
            break;
        
        case 0x5000: // SE Vx, Vy
            std::cout << "SE Vx, Vy" << std::endl;
            if (registers.get((opcode & 0x0f00)>>8) == registers.get((opcode & 0x00f0)>>4))
                pc += 2;
            break;
            
        case 0x6000: // LD Vx, byte
            std::cout << "LD Vx, byte" << std::endl;
            registers.set((opcode & 0x0f00)>>8, opcode & 0x00ff);
            break;
        
        case 0x7000: // ADD Vx, byte
            std::cout << "ADD Vx, byte" << std::endl;
            registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) + (opcode & 0x00ff));
            break;
        
        case 0x8000:
            switch (opcode & 0x000f)
            {
                case 0: // LD Vx, Vy
                    std::cout << "LD Vx, Vy" << std::endl;
                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x00f0)>>4));
                    break;
                
                case 1: // OR Vx, Vy
                    std::cout << "OR Vx, Vy" << std::endl;
                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) | registers.get((opcode & 0x00f0)>>4));
                    break;
                
                case 2: // AND Vx, Vy
                    std::cout << "AND Vx, Vy" << std::endl;
                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) & registers.get((opcode & 0x00f0)>>4));
                    break;
                
                case 3: // XOR Vx, Vy
                    std::cout << "XOR Vx, Vy" << std::endl;
                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) ^ registers.get((opcode & 0x00f0)>>4));
                    break;
                
                case 4: // ADD Vx, Vy
                    std::cout << "ADD Vx, Vy" << std::endl;
                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) + registers.get((opcode & 0x00f0)>>4));

                    if (registers.get((opcode & 0x00f0)>>4) > (0xff - registers.get((opcode && 0x0f00)>>8)))
                        registers.set(0xf, 1);
                    else
                        registers.set(0xf, 0);
                    break;
                
                case 5: // SUB Vx, Vy
                    std::cout << "SUB Vx, Vy" << std::endl;
                    registers.set(0xf, !(registers.get((opcode & 0x0f00)>>8) < registers.get((opcode & 0x00f0)>>4)));

                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) - registers.get((opcode & 0x00f0)>>4));
                    break;
                
                case 6: // SHR Vx {, Vy}
                    std::cout << "SHR Vx {, Vy}" << std::endl;
                    // Mark whether overflow occurs.
                    registers.set(0xf, registers.get((opcode & 0x0f00)>>8) & 1);

                    if (storeBitShiftResultOfY)
                        registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x00f0)>>4) >> 1);
                    else
                        registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) >> 1);
                    break;
                
                case 7: // SUBN Vx, Vy
                    std::cout << "SUBN Vx, Vy" << std::endl;
                    registers.set(0xf, !(registers.get((opcode & 0x0f00)>>8) > registers.get((opcode & 0x00f0)>>4)));

                    registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x00f0)>>4) - registers.get((opcode & 0x0f00)>>8));
                    break;
                
                case 0xe: // SHL Vx {, Vy}
                    std::cout << "SDL Vx, {, Vy}" << std::endl;
                    // Mark whether overflow occurs.
                    registers.set(0xf, (registers.get((opcode & 0x0f00)>>8) >> 7));

                    if (storeBitShiftResultOfY)
                        registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x00f0)>>4) << 1);
                    else
                        registers.set((opcode & 0x0f00)>>8, registers.get((opcode & 0x0f00)>>8) << 1);
                    break;
                
                default:
                    reportInvalidOpcode(opcode);
                    break;
            }
            break;
        
        case 0x9000: // SNE Vx, Vy
            std::cout << "SNE Vx, Vy" << std::endl;
            if (registers.get((opcode & 0x0f00)>>8) !=
                registers.get((opcode & 0x00f0)>>4))
                pc += 2;
            break;
        
        case 0xa000: // LD I, addr
            std::cout << "LD I, addr" << std::endl;
            I  = (opcode & 0x0fff);
            break;
        
        case 0xb000: // JP V0, addr
            std::cout << "JP V0, addr" << std::endl;
            pc = (registers.get(0) + (opcode & 0x0fff));
            break;
        
        case 0xc000: // RND Vx, byte
            std::cout << "RND Vx, byte" << std::endl;
            registers.set((opcode & 0x0f00)>>8, (opcode & 0x00ff) & static_cast<uint8_t>(std::rand()));
            break;
        
        case 0xd000: // DRW Vx, Vy, nibble
        {
            std::cout << "DRW Vx, Vy, nibble" << std::endl;
            
            int x{registers.get((opcode & 0x0f00) >> 8)};
            int y{registers.get((opcode & 0x00f0) >> 4)};
            int height{opcode & 0x000f};
            
            registers.set(0xf, 0);
            
            for (int cy{}; cy < height; ++cy)
            {
                uint8_t line{memory[I + cy]};
                
                for (int cx{}; cx < 8; ++cx)
                {
                    uint8_t pixel = line & (0x80 >> cx);
                    
                    if (pixel)
                    {
                        int index{(x + cx) + (y + cy)*64};
                        
                        if (frameBuffer[index])
                            registers.set(0xf, 1);
                            
                        frameBuffer[index] ^= 1;
                    }
                }
            }
            
            renderFlag = true;
            
            break;
        }
        
        case 0xe000:
            switch (opcode & 0x00ff)
            {
                case 0x9e: // SKP Vx
                {
                    std::cout << "SKP Vx" << std::endl;
                    
                    isReadingKey = true;

                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
                    std::cout << "KEY: " << keyState[keyMap[registers.get((opcode & 0x0f00)>>8)]] << std::endl;
               
                    if (keyState[keyMapScancode[registers.get((opcode & 0x0f00)>>8)]])
                        pc += 2;
                    break;
                }
                
                case 0xa1: // SKNP Vx
                {
                    std::cout << "SKNP Vx" << std::endl;
                    
                    isReadingKey = true;

                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
                    std::cout << "KEY: " << keyState[keyMap[registers.get((opcode & 0x0f00)>>8)]] << std::endl;
               
                    if (!(keyState[keyMapScancode[registers.get((opcode & 0x0f00)>>8)]]))
                        pc += 2;
                    break;
                }
                
                default:
                    reportInvalidOpcode(opcode);
                    break;
            }
        break;
        
        case 0xf000:
            switch (opcode & 0x00ff)
            {
                case 0x07: // LD Vx, DT
                    std::cout << "LD Vx, DT" << std::endl;
                    registers.set((opcode & 0x0f00)>>8, delayTimer);
                    break;
                
                case 0x0a: // LD Vx, K
                {
                    std::cout << "LD Vx, K" << std::endl;
                    
                    uint16_t pressedKey{};
                    
                    bool hasValidKeyPressed{false};
                    do
                    {
                        setDebugTitle();
                        
                        SDL_SetWindowTitle(window, 
                            (std::string(SDL_GetWindowTitle(window))+
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
                                	hasEnded = true;
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
                    
                    registers.set((opcode & 0x0f00)>>8, pressedKey);
                    
                    std::cout << "Loaded key: " << static_cast<int>(pressedKey) << std::endl;
                    
                    break;
                }
                
                case 0x15: // LD DT, Vx
                    std::cout << "LD DT, Vx" << std::endl;
                    delayTimer = registers.get((opcode & 0x0f00)>>8);
                    break;
                
                case 0x18: // LD ST, Vx
                    std::cout << "LD ST, Vx" << std::endl;
                    soundTimer = registers.get((opcode & 0x0f00)>>8);
                    break;
                
                case 0x1e: // ADD I, Vx
                    std::cout << "ADD I, Vx" << std::endl;
                    I += registers.get((opcode & 0x0f00)>>8);
                    break;
                
                case 0x29: // LD F, Vx
                    std::cout << "FD, F, Vx" << std::endl;
                    I = registers.get((opcode & 0x0f00)>>8)*5;
                    std::cout << "FONT LOADED: " << registers.get((opcode & 0x0f00)>>8) << std::endl;
                    break;
                
                case 0x33: // LD B, Vx
                {
                    std::cout << "LD B, Vx" << std::endl;
                    uint8_t number{registers.get((opcode & 0x0f00)>>8)};
                    memory[I] = (number / 100);
                    memory[I+1] = ((number / 10) % 10);
                    memory[I+2] = (number % 10);
                    break;
                }
                
                case 0x55: // LD [I], Vx
                {
                    std::cout << "LD [I], Vx" << std::endl;
                    uint8_t x{static_cast<uint8_t>((opcode & 0x0f00)>>8)};
                        
                    for (uint8_t i{}; i <= x; ++i)
                        memory[I + i] = registers.get(i);
                    
                    if (incrementIAfterMemoryOperation)
                        I += (x + 1);
                    break;
                }
                
                case 0x65: // LD Vx, [I]
                {
                    std::cout << "LD Vx, [I]" << std::endl;
                    uint8_t x{static_cast<uint8_t>((opcode & 0x0f00)>>8)};
                    
                    for (uint8_t i{}; i <= x; ++i)
                        registers.set(i, memory[I + i]);
                    
                    if (incrementIAfterMemoryOperation)
                        I += (x + 1);
                    break;
                }
                
                default:
                    reportInvalidOpcode(opcode);
                    break;
            }
        break;
            
        default:
            reportInvalidOpcode(opcode);
            break;
    }
    
    if (timerDecrementCountdown <= 0)
    {
        if (delayTimer > 0)
            --delayTimer;

        if (soundTimer > 0)
        {
            --soundTimer;
            Sound::makeBeepSound();
        }

        // reset the timer
        timerDecrementCountdown = 16.67;
    }
}
