#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer
{
public:
    using clock_t           = std::chrono::high_resolution_clock;
    //using nanoseconds_t     = std::chrono::nanoseconds;
    using milliseconds_t    = std::chrono::milliseconds;

    std::chrono::time_point<clock_t> beggining;

    void reset();
    int get();
};



#endif /* TIMER_H */
