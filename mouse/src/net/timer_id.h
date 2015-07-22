#ifndef MOUSE_NET_TIMERID_H
#define MOUSE_NET_TIMERID_H

namespace mouse
{

class Timer;

class TimerId
{
public:
    TimerId(Timer* timer = NULL, int64_t seq = 0)
        : timer_(timer),
          sequence_(seq)
    {
    }

    // default copy-ctor, dtor and assignment are okay

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t sequence_;
};

} //namespace Mouse

#endif
