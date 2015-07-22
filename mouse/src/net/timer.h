#ifndef MOUSE_NET_TIMER_H
#define MOUSE_NET_TIMER_H

#include <atomic>
#include <functional>

#include "../base/timestamp.h"
#include "callbacks.h"

namespace mouse
{

class Timer
{
    //nocopyable
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
public:

    Timer(const TimerCallback& cb, Timestamp when, double interval)
        : callback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(++s_num_created_)
    {
    }

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<int64_t> s_num_created_;
};

} //namespace Mouse

#endif
