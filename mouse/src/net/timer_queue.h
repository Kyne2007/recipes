#ifndef MOUSE_NET_TIMER_QUEUE_H
#define MOUSE_NET_TIMER_QUEUE_H

#include <functional>
#include <set>
#include <vector>

#include "../base/timestamp.h"
#include "callbacks.h"
#include "channel.h"

namespace mouse
{

class EventLoop;
class Timer;
class TimerId;

class TimerQueue
{
    //nocopyable
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);

    void cancel(TimerId timer_id);

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timer_id);
    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfd_channel_;
    // Timer list sorted by expiration
    TimerList timers_;

    // for cancel()
    bool calling_expired_timers_; /* atomic */
    ActiveTimerSet active_timers_;
    ActiveTimerSet canceling_timers_;
};

}//namespace mouse

#endif
