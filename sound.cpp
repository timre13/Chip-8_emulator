#include "sound.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <utilapiset.h>
#else
    #include <iostream>
    #include "ncurses.h"
#endif

void Sound::makeBeepSound()
{
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        Beep(4000, 100);
    #else
        if (beep()) // Try to beep using ncurses.
        {
            // If failed, try using the \a character.
            std::cout << '\a';
            std::cout.flush();
        }
    #endif
}
