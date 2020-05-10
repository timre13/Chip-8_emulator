#include "Timer.h"

void Timer::reset()
{
    beggining = clock_t::now();
}

int Timer::get()
{
    return std::chrono::duration_cast<milliseconds_t>(clock_t::now() - beggining).count();
}
