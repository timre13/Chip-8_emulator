#include <string>
#include <random>
#include <filesystem>

#include "config.h"
#include "Chip-8.h"
#include "sdl_file_chooser.h"
#include "sound.h"
#include "license.h"

#if !__has_include("submodules/chip8asm/src/version.h")
#error "Chip8asm submodule is missing. Run `git submodule init`."
#endif

#include "submodules/chip8asm/src/Logger.h"

int main(int argc, char** argv)
{
    std::cout << LICENSE_STR << std::endl;

    Logger::setLoggerVerbosity(Logger::LoggerVerbosity::Verbose);

    std::string romFilename{};
    if (argc > 1)
    {
        romFilename = argv[1];
    }
    else
    {
        FileChooser fileChooser{{"./roms", "../submodules/chip8asm/tests", "."}, {"ch8", "asm"}};
        romFilename = fileChooser.get();
        // If the user canceled the file selection or the file list is empty, quit.
        if (romFilename.size() == 0)
            return 0;
    }

    Logger::log << "Filename: " << romFilename << Logger::End;

    Chip8 chip8{romFilename};
    chip8.whenWindowResized(64 * 20, 32 * 20);

    Logger::log << std::hex;

    double emulationSpeed = 1.0;
    int frameDelay = 1000.0 / 500 / emulationSpeed;
    chip8.setSpeedPerc(100);

    bool isRunning = true;
    bool wasPaused{};
    bool isSteppingMode{};
    bool shouldStep{}; // no effect when not in stepping mode
    double renderUpdateCountdown{}; // Decremented and when 0, the renderer is updated

    while (isRunning && !chip8.hasExited())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    isRunning = false;
                    break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SHORTCUT_KEYCODE_PAUSE:
                            chip8.togglePause();
                            chip8.setInfoMessage(chip8.isPaused() ?
                                    Chip8::InfoMessageValue::Pause :
                                    Chip8::InfoMessageValue::Unpause);
                            isSteppingMode = false;
                            break;

                        case SHORTCUT_KEYCODE_QUIT:
                            isRunning = false;
                            break;

                        case SHORTCUT_KEYCODE_FULLSCREEN:
                            chip8.toggleFullscreen();
                            break;

                        case SHORTCUT_KEYCODE_DEBUG_MODE:
                            chip8.toggleDebugMode();
                            break;

                        case SHORTCUT_KEYCODE_TOGGLE_CURSOR:
                            chip8.toggleCursor();
                            break;

                        case SHORTCUT_KEYCODE_STEP_INST:
                            shouldStep = true;
                            break;

                        case SHORTCUT_KEYCODE_STEPPING_MODE:
                            isSteppingMode = !isSteppingMode;
                            chip8.unpause();
                            chip8.setInfoMessage(isSteppingMode ?
                                    Chip8::InfoMessageValue::EnableSteppingMode :
                                    Chip8::InfoMessageValue::DisableSteppingMode);
                            break;

                        case SHORTCUT_KEYCODE_DUMP_STATE:
                            Logger::log << '\n' << chip8.dumpStateToStr() << Logger::End;
                            chip8.setInfoMessage(Chip8::InfoMessageValue::DumpState);
                            break;

                        case SHORTCUT_KEYCODE_INC_SPEED:
                            emulationSpeed += 0.05;
                            if (emulationSpeed > 10)
                                emulationSpeed = 10;
                            frameDelay = 1000.0 / 500 / emulationSpeed;
                            chip8.setSpeedPerc(emulationSpeed * 100);
                            chip8.setInfoMessage(Chip8::InfoMessageValue::IncrementSpeed);
                            break;

                        case SHORTCUT_KEYCODE_DEC_SPEED:
                            emulationSpeed -= 0.05;
                            if (emulationSpeed < 0.05)
                                emulationSpeed = 0.05;
                            frameDelay = 1000.0 / 500 / emulationSpeed;
                            chip8.setSpeedPerc(emulationSpeed * 100);
                            chip8.setInfoMessage(Chip8::InfoMessageValue::DecrementSpeed);
                            break;

                        case SHORTCUT_KEYCODE_RESET:
                            chip8.reset();
                            chip8.setInfoMessage(Chip8::InfoMessageValue::Reset);
                            break;

                        case SHORTCUT_KEYCODE_SCREENSHOT:
                            chip8.setInfoMessage(Chip8::InfoMessageValue::Screenshot, chip8.saveScreenshot());
                            break;

                        case SHORTCUT_KEYCODE_TOGGLE_HELP:
                            chip8.toggleKeyboardHelp();
                            break;

                        case SHORTCUT_KEYCODE_TOGGLE_COMPAT_SHIFTYREG:
                            chip8.toggleCompatShiftYRegInsteadOfX();
                            chip8.setInfoMessage(Chip8::InfoMessageValue::ToggleCompatShiftYRegInsteadOfX);
                            break;

                        case SHORTCUT_KEYCODE_TOGGLE_COMPAT_INCI:
                            chip8.toggleCompatIncIAfterRegFillLoad();
                            chip8.setInfoMessage(Chip8::InfoMessageValue::ToggleCompatIncIAfterRegFillLoad);
                            break;
                    }
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.windowID == chip8.getWindowID())
                    {
                        switch (event.window.event)
                        {
                        case SDL_WINDOWEVENT_RESIZED:
                            chip8.whenWindowResized(event.window.data1, event.window.data2);
                            continue;
                            break;

                        case SDL_WINDOWEVENT_CLOSE:
                            isRunning = false;
                            continue;
                            break;
                        }
                    }
                    break;
            }
        }

        if (chip8.isPaused() || (isSteppingMode && !shouldStep))
        {
            wasPaused = true;

            chip8.renderFrameBuffer();
            chip8.renderDebugInfoIfInDebugMode();
            chip8.copyTexturesToRenderer();
            chip8.updateInfoMessage();
            chip8.updateOverlay();
            chip8.updateRenderer();

            SDL_Delay(frameDelay);

            // If paused or the stepping key was not pressed, don't execute instruction
            if (chip8.isPaused() || !shouldStep)
                continue;
        }

        if (wasPaused)
        {
            // To make the pixels light again.
            chip8.renderFrameBuffer();

            wasPaused = false;

            // We don't need a redraw for a while
            renderUpdateCountdown = 16.67;
        }

        chip8.clearLastRegisterOperationFlags();
        chip8.clearIsReadingKeyStateFlag();

        chip8.emulateCycle();

        // Mark that we executed an instruction since the last step
        shouldStep = false;

        if (chip8.getRenderFlag() || renderUpdateCountdown <= 0)
        {
            chip8.renderFrameBuffer();
            renderUpdateCountdown = 16.67;
        }

        chip8.renderDebugInfoIfInDebugMode();
        chip8.copyTexturesToRenderer();
        chip8.updateInfoMessage();
        chip8.updateOverlay();
        chip8.updateRenderer();

        SDL_Delay(frameDelay);

        renderUpdateCountdown -= frameDelay;
    }

    chip8.deinit();
}
