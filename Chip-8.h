#ifndef CHIP8_H
#define CHIP8_H

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <fstream>
#include <stdint.h>
#include <cassert>
#include <bitset>

#include "config.h"
#include "to_hex.h"
#include "sound.h"
#include "submodules/chip8asm/src/Logger.h"

#define TITLE "CHIP-8 Emulator"

class Registers final
{
private:
    uint8_t m_registers[16]{};

    bool m_isRegisterWritten[16]{};
    bool m_isRegisterRead[16]{};

public:
    uint8_t get(int index, bool isInternal=false)
    {
        assert(index >= 0);
        assert(index < 16);

        if (!isInternal)
            m_isRegisterRead[index] = true;

        return m_registers[index];
    }

    void set(int index, uint8_t value, bool isInternal=false)
    {
        assert(index >= 0);
        assert(index < 16);

        if (!isInternal)
            m_isRegisterWritten[index] = true;

        m_registers[index] = value;
    }

    void clearReadWrittenFlags()
    {
        // Clear m_isRegisterWritten
        memset(m_isRegisterWritten, false, sizeof(m_isRegisterWritten));
        // Clear m_isRegisterRead
        memset(m_isRegisterRead, false, sizeof(m_isRegisterRead));
    }

    inline bool getIsRegisterWritten(int index) const
    {
        return m_isRegisterWritten[index];
    }

    inline bool getIsRegisterRead(int index) const
    {
        return m_isRegisterRead[index];
    }
};

class Framebuffer final
{
public:
    int m_frameBuffer[64 * 32]{};

    Framebuffer()
    {
    }

    void set(int x, int y, int val)
    {
        assert(x >= 0 && x < 64);
        assert(y >= 0 && y < 32);
        m_frameBuffer[y*64+x] = val;
    }

    int get(int x, int y) const
    {
        assert(x >= 0 && x < 64);
        assert(y >= 0 && y < 32);
        return m_frameBuffer[y*64+x];
    }

    int get(int index) const
    {
        assert(index >= 0 && index < 64*32);
        return m_frameBuffer[index];
    }

    void clear()
    {
        for (int i{}; i < 64*32; ++i)
        {
            m_frameBuffer[i] = 0;
        }
    }

    void print()
    {
        Logger::log << "--- frame buffer ---\n";
        for (int i{}; i < 64 * 32; ++i)
        {
            Logger::log << m_frameBuffer[i];
            if ((i + 1) % 64 == 0)
                Logger::log << '\n';
        }
        Logger::log << "--------------------" << Logger::End;
    }
};

class Chip8 final
{
public:
    enum class InfoMessageValue
    {
        None,
        Pause,
        Unpause,
        Reset,
        Screenshot,
        EnableSteppingMode,
        DisableSteppingMode,
        DecrementSpeed,
        IncrementSpeed,
        DumpState,
        ToggleCompatShiftYRegInsteadOfX,
        ToggleCompatIncIAfterRegFillLoad,
    };

private:
    // stack
    uint16_t m_stack[16]{};
    // stack pointer (4 bits)
    uint8_t m_sp: 4; // It will be init-ed in ctor
    // registers
    Registers m_registers;
    // memory - 0x00 - 0xfff
    uint8_t m_memory[0xfff+1]{};
    // program counter - the programs start at 0x200
    uint16_t m_pc = 0x200;
    // current opcode
    uint16_t m_opcode = 0;
    // index register
    uint16_t m_indexReg = 0;
    // delay timer
    uint8_t m_delayTimer = 0;
    // sound timer
    uint8_t m_soundTimer = 0;
    // framebuffer - stores which pixels are turned on
    // We don't fill it with zeros, because the original implementation doesn't do so
    Framebuffer m_frameBuffer;

    std::string m_romFilename;
    // rom file size in bytes
    int m_romSize;

    SDL_Window* m_window{};
    int m_windowWidth{};
    int m_windowHeight{};
    SDL_Renderer* m_renderer{};

    // A texture where we render the game
    SDL_Texture* m_contentTexture{};
    // The texture of the debugger window
    SDL_Texture* m_debuggerTexture{};

    Beeper m_beeper;
    int m_remainingBeepFrames{};

    // Every character from code 21 to code 126 prerendered
    SDL_Texture* m_fontCache['~' - '!' + 1]{};

    int m_scale = 1;
    bool m_isFullscreen{};
    bool m_isDebugMode{};
    bool m_isPaused{};
    bool m_isReadingKey{};

    // Helps to decrement the sound and delay timers at 60 FPS
    // This is decremented after every frame and if 0, the timers decremented.
    double m_timerDecrementCountdown = 16.67;

    bool m_hasDeinitCalled{};

    // Whether the program should exit
    bool m_hasExited{};
    // Marks whether we need to redraw the framebuffer
    bool m_renderFlag = true;

    int m_emulSpeedPerc{};
    int m_frameDelay{};

    InfoMessageValue m_infoMessage{};
    std::string m_infoMessageExtra;
    float m_infoMessageTimeRemaining{};

    bool m_shouldShowKeyboardHelp{};

    /*
     * The `8xy6` opcode is right-shift, the `8xyE` is left-shift.
     * Register 0xF is set to the shifted-out bit of register X.
     *
     * If this is true,
     *      register X is set to register Y shifted,
     * if false,
     *      register X is set to register X shifted.
     *
     * The old implementations used the Y register.
     */
    bool m_compat_shiftYRegInsteadOfX = true;

    /*
     * The `Fx55` and `Fx66` opcodes loop through the registers and write them to / read from the memory.
     * This variable marks if the index register needs to be incremented while doing the operations.
     * In the original implementation this does happen.
     */
    bool m_compat_incIAfterRegFillLoad = true;


    void loadFontSet();
    void initVideo();

    void fetchOpcode();

    /*
     * Should be called when a serious error happens.
     * Displays some info, waits for escape key and `abort()`s.
     */
    [[noreturn]] void panic(const std::string& message);

public:
    Chip8(const std::string& romFilename);

    void reset(bool reloadFile=true);
    void loadFile(const std::string& romFilename);

    void emulateCycle();
    void renderFrameBuffer();

    inline void setSpeedPerc(int value)
    {
        m_frameDelay = 1000.0 / 500 / (value / 100.0);
        m_emulSpeedPerc = value;
        updateWindowTitle();
    }

    void copyTexturesToRenderer();
    inline void updateRenderer() { SDL_RenderPresent(m_renderer); }

    void updateWindowTitle();

    inline void togglePause() { m_isPaused = !m_isPaused; updateWindowTitle(); }
    inline void pause() { m_isPaused = true; updateWindowTitle(); }
    inline void unpause() { m_isPaused = false; updateWindowTitle(); }
    inline bool isPaused() const { return m_isPaused; }

    void whenWindowResized(int width, int height);

    void toggleFullscreen();
    void toggleDebugMode();
    inline void toggleCursor() { SDL_ShowCursor(!SDL_ShowCursor(-1)); }
    void toggleCompatShiftYRegInsteadOfX();
    void toggleCompatIncIAfterRegFillLoad();

    void renderDebugInfoIfInDebugMode();

    inline uint32_t getWindowID() const { return SDL_GetWindowID(m_window); }

    inline bool hasExited() const { return m_hasExited; }
    inline bool getRenderFlag() const { return m_renderer; }

    inline void clearLastRegisterOperationFlags() { m_registers.clearReadWrittenFlags(); }
    inline void clearIsReadingKeyStateFlag() { m_isReadingKey = false; }

    /*
     * If `dumpAll` is true, the memory and the screenbuffer are dumped, too.
     */
    std::string dumpStateToStr(bool dumpAll=true);

    std::string saveScreenshot() const;

    inline void setInfoMessage(InfoMessageValue message, const std::string& extra="")
    {
        m_infoMessage = message;
        m_infoMessageExtra = extra;
        m_infoMessageTimeRemaining = MESSAGE_SHOW_TIME_S;
    }
    void updateInfoMessage();

    inline void toggleKeyboardHelp() { m_shouldShowKeyboardHelp = !m_shouldShowKeyboardHelp; }
    void updateOverlay();

    void deinit();
    ~Chip8();
};

#endif // CHIP8_H
