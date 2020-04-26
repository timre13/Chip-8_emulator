#ifndef CHIP8_H
#define CHIP8_H

#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <stdint.h>
#include <cassert>
#include <bitset>

#define TITLE "CHIP-8 Emulator"

extern bool storeBitShiftResultInY;
extern bool incrementIAfterMemoryOperation;

extern uint8_t bgColorR;
extern uint8_t bgColorG;
extern uint8_t bgColorB;

extern uint8_t fgColorR;
extern uint8_t fgColorG;
extern uint8_t fgColorb;


class Registers
{
private:
    uint8_t m_registers[16]{};

public:
    uint8_t& operator[](int index)
    {
        assert(index >= 0);
        assert(index < 16);
        
        std::cout << "register " << index << ": " << static_cast<int>(m_registers[index]) << std::endl;
        
        return m_registers[index];
    }
};

class Framebuffer
{
public:
    int m_frameBuffer[64*32]{};
    
    Framebuffer()
    {}

    int& operator[](int index)
    {
        //std::cout << index << " element of frame buffer accessed" << std::endl;
        
        //assert(index >= 0);
        //assert(index < 32);
        
        bool isOutOfBounds{false};
        
        if (index < 0)
        {
            isOutOfBounds = true;
            std::cout << "Frame buffer index lower than 0" << std::endl;
        }
        if (index >= 64*32)
        {
            isOutOfBounds = true;
            std::cout << "Frame buffer index out of bounds" << std::endl;
        }
        
        if (isOutOfBounds)
            return m_frameBuffer[0];
        
        return m_frameBuffer[index];
    }
    
    void print()
    {
        std::cout << "--- frame buffer ---" << std::endl;
        for (int i{}; i < 64*32; ++i)
        {
            std::cout << m_frameBuffer[i];
            if (!((i+1) % 64))
                std::cout << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
};

class Chip8
{
private:
    // stack
    uint16_t stack[16]{};
    // stack pointer
    uint8_t sp          = -1;
    // registers
    Registers registers;
    // memory - 0x00 - 0xFFF
    uint8_t memory[0xfff+1]{};
    // program counter - the programs start at 0x200
    uint16_t pc         = 0x200;
    // current opcode
    uint16_t opcode     = 0;
    // index register
    uint16_t I          = 0;
    // delay timer
    uint8_t delayTimer  = 0;
    // sound timer
    uint8_t soundTimer  = 0;
    // framebuffer - stores which pixels are turned on
    // We don't fill it with zeros, because the original implementation doesn't do so
    Framebuffer frameBuffer;

    //std::fstream romFile;
    int romSize;

    SDL_Window *window{nullptr};
    SDL_Renderer *renderer{nullptr};

    double scale{1.0};

    bool hasDeinitCalled{false};

    
    void loadFile(std::string romFilename);
    void loadFontSet();
    void initVideo();
    
    void fetchOpcode();
    
public:
    Chip8(const std::string &romFilename);
    ~Chip8();
    
    void deinit();

    void emulateCycle();
    void renderFrameBuffer();
    
    void updateRenderer();
    
    void setDebugTitle();
    void setPaused();
    
    void whenWindowResized(int width, int height);

    uint32_t getWindowID();

    bool hasEnded{false}; // marks whether the program ended
    // Marks whether we need to redraw the framebuffer
    bool renderFlag = true;
};

#endif // CHIP8_H
