#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <random>
#include <ctime>

#include "Chip-8.h"
#include "fontset.h"

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
    
    std::srand(std::time(nullptr));
    std::rand();
    
    std::cout << '\n' << "----- setting up video -----" << std::endl;
    Chip8::initVideo();
    Chip8::loadFontSet();
    std::cout << '\n' << "----- loading file -----" << std::endl;
    Chip8::loadFile(romFilename);
}

void Chip8::loadFile(std::string romFilename)
{
    std::cout << "Memory size: " << sizeof(memory) / sizeof(memory[0]) << std::endl;
    
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
    
    std::cout << "End of program segment: " << std::dec <<  511+romSize << std::hex << '\n';
    
    uint8_t *buffer = static_cast<uint8_t *>(malloc(8 * romSize));
    
    if (buffer == nullptr)
    {
        std::cout << "Unable to allocate memory" << '\n';
        std::exit(2);
    }
    
    for (int i{}; i < romSize; ++i)
        buffer[i] = 0;
    
    auto copied = fread(buffer, 8, romSize, romFile);
    
    std::cout << "Copied: " << copied << std::endl;
    
    if (!copied)
    {
        std::cout << "Unable to copy to buffer" << '\n';
        std::exit(2);
    }
    
    std::cout << '\n' << "--- BUFFER --- " << '\n';
    for (int i{}; i < romSize; ++i)
        std::cout << static_cast<int>(buffer[i]) << ' ';
    std::cout << '\n' << "--- END OF BUFFER ---" << '\n';
    
    
    for (int i{}; i < romSize; ++i)
    {
        memory[512+i] = buffer[i];
    }
    
    for (int i{}; i < 0xfff+1; ++i)
    {
        std::cout << std::hex << static_cast<int>(memory[i]) << ' ';
        if ((i == 0x200-1) || (i == 0xFFF))
            std::cout << '\n' << std::string(170, '-') << '\n';
        if ((i) == (romSize+511))
            std::cout << '\n' << "-- END OF PROGRAM ---" << '\n';
    }
    std::cout << '\n';
    
    fclose(romFile);
    free(buffer);
}

void Chip8::loadFontSet()
{
    std::cout << '\n' << "--- FONT SET --- " << '\n';
    for (int i{}; i < 80; ++i)
        std::cout << static_cast<int>(fontset[i]) << ' ';
    std::cout << '\n' << "--- END OF FONT SET ---" << '\n';
    
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
    
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    SDL_ShowCursor(SDL_DISABLE);

    SDL_SetWindowMinimumSize(window, 200, 100);
}

void Chip8::deinit()
{
    std::cout << '\n' << "----- deinit -----" << std::endl;
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
       
    SDL_Quit();
}

Chip8::~Chip8()
{
    deinit();
}

void Chip8::renderFrameBuffer()
{
    SDL_SetRenderDrawColor(renderer, bgColorR, bgColorG, bgColorB, 255);
    SDL_RenderClear(renderer);

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
    
    SDL_RenderPresent(renderer);
    
    renderFlag = false;
    
    std::cout << "Frame rendered" << std::endl;
}

void Chip8::updateRenderer()
{
    SDL_RenderPresent(renderer);
}

void Chip8::fetchOpcode()
{
    // Catch the access out of the valid memory address range (0x00 - 0xFFF)
    assert(pc <= 0xfff);
    
    if (pc > 0xfff)
    {
        std::cout << "Memory accessed out of range" << std::endl;
        hasEnded = true;
        return;
    }
    
    if (pc >= romSize+512)
    {
        std::cout << "Program ended" << std::endl;
        hasEnded = true;
        return;
    }
    
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
    SDL_SetWindowTitle(window,
        (std::string(TITLE)+" - "+
            "PC: "      + std::to_string(pc)         + " "
            "I: "       + std::to_string(I)          + " "
            "SP: "      + std::to_string(sp)         + " "
            "DT: "      + std::to_string(delayTimer) + " "
            "ST: "      + std::to_string(soundTimer) + " "
            "Opcode: "  + std::to_string(opcode)     + " "
        ).c_str());
}

void Chip8::setPaused()
{
	SDL_SetWindowTitle(window, (std::string(TITLE)+" - [PAUSED]").c_str());

	SDL_Rect rect{0,
				  0,
				  static_cast<int>(std::ceil(64*20*scale)),
				  static_cast<int>(std::ceil(32*20*scale))
	};

	SDL_BlendMode originalBlendmode{};
	SDL_GetRenderDrawBlendMode(renderer, &originalBlendmode);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_SetRenderDrawColor(renderer, bgColorR, bgColorG, bgColorB, 150);

	SDL_RenderFillRect(renderer, &rect);

	SDL_RenderPresent(renderer);

	SDL_SetRenderDrawBlendMode(renderer, originalBlendmode);
}

void Chip8::whenWindowResized(int width, int height)
{
	std::cout << "Window resized" << '\n';

	double horizontalScale{width  / (64*20.0)};
	double   verticalScale{height / (32*20.0)};

	scale = std::min(horizontalScale, verticalScale);

	renderFrameBuffer();
}

uint32_t Chip8::getWindowID()
{
	return SDL_GetWindowID(window);
}

void Chip8::emulateCycle()
{
    fetchOpcode();

    setDebugTitle();
    
    std::cout << std::hex;
    
    switch (opcode & 0xF000)
    {
        case 0x0000:
            switch (opcode & 0x0FFF)
            {
                case 0x0000:
                    std::cout << "NOP" << std::endl;
                    break;
                    
                case 0x00E0: // CLS
                    std::cout << "CLS" << std::endl;
                    for (int i{}; i < 64*32; ++i)
                        frameBuffer[i] = 0;
                    renderFlag = true;
                    break;
                
                case 0x00EE: // RET
                    std::cout << "RET" << std::endl;
                    pc = stack[sp];
                    --sp;
                    break;
                    
                default:
                    std::cout << "UNKNOWN OPCODE STARTING WITH 0" << std::endl;
                    break;
            }
            break;
            
        case 0x1000: // JMP
            std::cout << "JMP" << std::endl;
            pc = opcode & 0x0FFF;
            break;
            
        case 0x2000: // CALL
            std::cout << "CALL" << std::endl;
            ++sp;
            stack[sp] = pc;
            pc = (opcode & 0x0FFF);
            break;
            
        case 0x3000: // SE
            std::cout << "SE" << std::endl;
            if (registers[(opcode & 0x0F00)>>8] == (opcode & 0x00FF))
                pc += 2;
            break;
            
        case 0x4000: // SNE
            std::cout << "SNE" << std::endl;
            if (registers[(opcode & 0x0F00)>>8] != (opcode & 0x00FF))
                pc += 2;
            break;
        
        case 0x5000: // SE Vx, Vy
            std::cout << "SE Vx, Vy" << std::endl;
            if (registers[(opcode & 0x0F00)>>8] == registers[(opcode & 0x00F0)>>4])
                pc += 2;
            break;
            
        case 0x6000: // LD Vx, byte
            std::cout << "LD Vx, byte" << std::endl;
            registers[(opcode & 0x0F00)>>8] = (opcode & 0x00FF);
            break;
        
        case 0x7000: // ADD Vx, byte
            std::cout << "ADD Vx, byte" << std::endl;
            registers[(opcode & 0x0F00)>>8] += (opcode & 0x00FF);
            break;
        
        case 0x8000:
            switch (opcode & 0x000F)
            {
                case 0: // LD Vx, Vy
                    std::cout << "LD Vx, Vy" << std::endl;
                    registers[(opcode & 0x0F00)>>8] = registers[(opcode & 0x00F0)>>4];
                    break;
                
                case 1: // OR Vx, Vy
                    std::cout << "OR Vx, Vy" << std::endl;
                    registers[(opcode & 0x0F00)>>8] |= registers[(opcode & 0x00F0)>>4];
                    break;
                
                case 2: // AND Vx, Vy
                    std::cout << "AND Vx, Vy" << std::endl;
                    registers[(opcode & 0x0F00)>>8] &= registers[(opcode & 0x00F0)>>4];
                    break;
                
                case 3: // XOR Vx, Vy
                    std::cout << "XOR Vx, Vy" << std::endl;
                    registers[(opcode & 0x0F00)>>8] ^= registers[(opcode & 0x00F0)>>4];
                    break;
                
                case 4: // ADD Vx, Vy
                    std::cout << "ADD Vx, Vy" << std::endl;
                    registers[(opcode & 0x0F00)>>8] += registers[(opcode & 0x00F0)>>4];
                    if (registers[(opcode & 0x00F0)>>4] > (0xFF - registers[(opcode && 0x0F00)>>8]))
                        registers[0xF] = 1;
                    else
                        registers[0xF] = 0;
                    break;
                
                case 5: // SUB Vx, Vy
                    std::cout << "SUB Vx, Vy" << std::endl;
                    registers[0xF] = !(registers[(opcode & 0x0F00)>>8] < registers[(opcode & 0x00F0)>>4]);
                    registers[(opcode & 0x0F00)>>8] -= registers[(opcode & 0x00F0)>>4];
                    break;
                
                case 6: // SHR Vx {, Vy}
                    std::cout << "SHR Vx {, Vy}" << std::endl;
                    registers[0xF] = ((registers[(opcode & 0x0F00)>>8]) & 1);
                    if (storeBitShiftResultInY)
                        registers[(opcode & 0x00F0)>>4] = (registers[(opcode & 0x0F00)>>8] >> 1);
                    else
                        registers[(opcode & 0x0F00)>>8] >>= 1;
                    break;
                
                case 7: // SUBN Vx, Vy
                    std::cout << "SUBN Vx, Vy" << std::endl;
                    registers[0xF] = !(registers[(opcode & 0x0F00)>>8] > registers[(opcode & 0x00F0)>>4]);
                    registers[(opcode & 0x0F00)>>8] = (registers[(opcode & 0x00F0)>>4] - registers[(opcode & 0x0F00)>>8]);
                    break;
                
                case 0xE: // SHL Vx {, Vy}
                    std::cout << "SDL Vx, {, Vy}" << std::endl;
                    registers[0xF] = (registers[((opcode & 0x0F00)>>8)] >> 7);
                    if (storeBitShiftResultInY)
                        registers[(opcode & 0x00F0)>>4] = (registers[(opcode & 0x0F00)>>8] << 1);
                    else
                        registers[(opcode & 0x0F00)>>8] <<= 1;
                    break;
                
                default:
                    std::cout << "UNKNOWN OPCODE STARTING WITH 8" << std::endl;
                    break;
            }
            break;
        
        case 0x9000: // SNE Vx, Vy
            std::cout << "SNE Vx, Vy" << std::endl;
            if (registers[(opcode & 0x0F00)>>8] !=
                registers[(opcode & 0x00F0)>>4])
                pc += 2;
            break;
        
        case 0xA000: // LD I, addr
            std::cout << "LD I, addr" << std::endl;
            I  = (opcode & 0x0FFF);
            break;
        
        case 0xB000: // JP V0, addr
            std::cout << "JP V0, addr" << std::endl;
            pc = (registers[0] + (opcode & 0x0FFF));
            break;
        
        case 0xC000: // RND Vx, byte
            std::cout << "RND Vx, byte" << std::endl;
            registers[(opcode & 0x0F00)>>8] = ((opcode & 0x00FF) & static_cast<uint8_t>(std::rand()));
            break;
        
        case 0xD000: // DRW Vx, Vy, nibble
        {
            std::cout << "DRW Vx, Vy, nibble" << std::endl;
            
            int x{registers[(opcode & 0x0F00) >> 8]};
            int y{registers[(opcode & 0x00F0) >> 4]};
            int height{opcode & 0x000F};
            
            registers[0xF] = 0;
            
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
                            registers[0xF] = 1;
                            
                        frameBuffer[index] ^= 1;
                    }
                }
            }
            
            renderFlag = true;
            
            break;
        }
        
        case 0xE000:
            switch (opcode & 0x00FF)
            {
                case 0x9E: // SKP Vx
                {
                    std::cout << "SKP Vx" << std::endl;
                    
                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
                    std::cout << "KEY: " << keyState[keyMap[registers[(opcode & 0x0F00)>>8]]] << std::endl;
               
                    if (keyState[keyMapScancode[registers[(opcode & 0x0F00)>>8]]])
                        pc += 2;
                    break;
                }
                
                case 0xA1: // SKNP Vx
                {
                    std::cout << "SKNP Vx" << std::endl;
                    
                    auto keyState{SDL_GetKeyboardState(nullptr)};
                    
                    std::cout << "KEY: " << keyState[keyMap[registers[(opcode & 0x0F00)>>8]]] << std::endl;
               
                    if (!(keyState[keyMapScancode[registers[(opcode & 0x0F00)>>8]]]))
                        pc += 2;
                    break;
                }
                
                default:
                    std::cout << "UNKNOWN OPCODE STARTING WITH E" << std::endl;
                    break;
            }
        break;
        
        case 0xF000:
            switch (opcode & 0x00FF)
            {
                case 0x07: // LD Vx, DT
                    std::cout << "LD Vx, DT" << std::endl;
                    registers[(opcode & 0x0F00)>>8] = delayTimer;
                    break;
                
                case 0x0A: // LD Vx, K
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
                        
                        SDL_RenderPresent(renderer);
                        
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
                    
                    registers[(opcode & 0x0F00)>>8] = pressedKey;
                    
                    std::cout << "Loaded key: " << static_cast<int>(pressedKey) << std::endl;
                    
                    break;
                }
                
                case 0x15: // LD DT, Vx
                    std::cout << "LD DT, Vx" << std::endl;
                    delayTimer = registers[(opcode & 0x0F00)>>8];
                    break;
                
                case 0x18: // LD ST, Vx
                    std::cout << "LD ST, Vx" << std::endl;
                    soundTimer = registers[(opcode & 0x0F00)>>8];
                    break;
                
                case 0x1E: // ADD I, Vx
                    std::cout << "ADD I, Vx" << std::endl;
                    I += registers[(opcode & 0x0F00)>>8];
                    break;
                
                case 0x29: // LD F, Vx
                    std::cout << "FD, F, Vx" << std::endl;
                    std::cout << "FONT LOADED: " << registers[(opcode & 0x0F00)>>8] << std::endl;
                    I = registers[(opcode & 0x0F00)>>8]*5;
                    break;
                
                case 0x33: // LD B, Vx
                {
                    std::cout << "LD B, Vx" << std::endl;
                    uint8_t number{registers[(opcode & 0x0F00)>>8]};
                    memory[I] = (number / 100);
                    memory[I+1] = ((number / 10) % 10);
                    memory[I+2] = (number % 10);
                    break;
                }
                
                case 0x55: // LD [I], Vx
                {
                    std::cout << "LD [I], Vx" << std::endl;
                    uint8_t x{static_cast<uint8_t>((opcode & 0x0F00)>>8)};
                        
                    for (uint8_t i{}; i <= x; ++i)
                        memory[I + i] = registers[i];
                    
                    if (incrementIAfterMemoryOperation)
                        I += (x + 1);
                    break;
                }
                
                case 0x65: // LD Vx, [I]
                {
                    std::cout << "LD Vx, [I]" << std::endl;
                    uint8_t x{static_cast<uint8_t>((opcode & 0x0F00)>>8)};
                    
                    for (uint8_t i{}; i <= x; ++i)
                        registers[i] = memory[I + i];
                    
                    if (incrementIAfterMemoryOperation)
                        I += (x + 1);
                    break;
                }
                
                default:
                    std::cout << "UNKNOWN OPCODE STARTING WITH F" << std::endl;
                    break;
            }
        break;
            
        default:
            std::cout << "Unknown opcode: " << opcode << std::endl;
            break;
    }
    
    if (delayTimer > 0)
        --delayTimer;
    
    if (soundTimer > 0)
    {
        --soundTimer;
        // TODO: sound
    }
}
