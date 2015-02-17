#include "timer.h"


Timer::Timer(const TimerCallback& cb, Timestamp when, double interval):
    callback_(cb),
    expiration_(when),
    interval_(interval),
    repeat_(interval_ > 0)
{

}

Timer::~Timer() {

}
