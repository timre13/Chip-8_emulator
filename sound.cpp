#include "sound.h"

#include "./submodules/chip8asm/src/Logger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#define BEEP_AMPLITUDE 2800
#define BEEP_SAMPLE_RATE 44100
#define BEEP_FREQ 1500.0f

static void audioCallback(void *userData, Uint8* _buffer, int byteCount)
{
    Sint16* buffer = (Sint16*)_buffer;
    const int length = byteCount/2; // 2 bytes per sample
    int &sampleI = *(int*)userData;

    for (int i{}; i < length; ++i, ++sampleI)
    {
        const double time = (double)sampleI/BEEP_SAMPLE_RATE;
        buffer[i] = BEEP_AMPLITUDE*sin(M_PI*2*time*BEEP_FREQ);
    }
}

Beeper::Beeper()
{
    m_couldInit = false;
    m_sampleI = 0;

    SDL_AudioSpec want;
    want.freq = BEEP_SAMPLE_RATE;
    want.format = AUDIO_S16SYS; // Signed 16-bit sample type
    want.channels = 1;
    want.samples = 2048;
    want.callback = audioCallback; // SDL calls it to refill the buffer
    want.userdata = (void*)&m_sampleI;

    SDL_AudioSpec have;
    if (SDL_OpenAudio(&want, &have))
    {
        Logger::err << "Failed to open audio: " << SDL_GetError() << Logger::End;
        return;
    }
    if (want.format != have.format)
    {
        Logger::err << "Failed to get desired AudioSpec: " << SDL_GetError() << Logger::End;
        return;
    }

    Logger::log << "Opened and set up audio device" << Logger::End;
    m_couldInit = true;
}

void Beeper::startBeeping() const
{
    if (!m_couldInit)
        return;

    SDL_PauseAudio(0);
}

void Beeper::stopBeeping() const
{
    if (!m_couldInit)
        return;

    SDL_PauseAudio(1);
}

Beeper::~Beeper()
{
    if (!m_couldInit)
        return;

    SDL_CloseAudio();
    Logger::log << "Closed audio device" << Logger::End;
}
