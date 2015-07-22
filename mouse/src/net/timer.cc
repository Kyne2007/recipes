#include "timer.h"

using namespace mouse;

std::atomic<int64_t> Timer::s_num_created_;

void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}
