#include "sound.h"

#include <iostream>
#include "ncurses.h"

void Sound::makeBeepSound()
{
    if (beep()) // Try to beep using ncurses.
    {
        // If failed, try using the \a character.
        std::cout << '\a';
        std::cout.flush();
    }
}
