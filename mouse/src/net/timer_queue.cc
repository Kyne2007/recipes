#include "timer_queue.h"

#include "event_loop.h"
#include "timer.h"
#include "timer_id.h"

#include <glog/logging.h>
#include <sys/timerfd.h>
#include <strings.h>
#include <assert.h>

namespace mouse
{

int createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerfd < 0)
    {
        LOG(FATAL) << "Failed in timerfd_create";
    }

    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microsecondsSinceEpoch()
        - Timestamp::now().microsecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    DLOG(INFO) << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    {
        LOG(ERROR) << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    // wake up loop by timerfd_settime()
    struct itimerspec new_value;
    struct itimerspec old_value;
    bzero(&new_value, sizeof new_value);
    bzero(&old_value, sizeof old_value);
    new_value.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret)
    {
        LOG(ERROR) << "timerfd_settime()";
    }
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfd_channel_(loop, timerfd_),
      timers_(),
      calling_expired_timers_(false)
{
    timerfd_channel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfd_channel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (TimerList::iterator it = timers_.begin();
            it != timers_.end(); ++it)
    {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval)
{
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer);
}

void TimerQueue::cancel(TimerId timer_id)
{
    loop_->runInLoop(
            std::bind(&TimerQueue::cancelInLoop, this, timer_id));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->assertInLoopThread();
    bool earliest_changed = insert(timer);

    if (earliest_changed)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timer_id)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == active_timers_.size());

    ActiveTimer timer(timer_id.timer_, timer_id.sequence_);
    ActiveTimerSet::iterator it = active_timers_.find(timer);
    if (it != active_timers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); (void)n;
        delete it->first; // FIXME: no delete please
        active_timers_.erase(it);
    }
    else if (calling_expired_timers_)
    {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    calling_expired_timers_ = true;
    canceling_timers_.clear();

    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
            it != expired.end(); ++it)
    {
        it->second->run();
    }
    calling_expired_timers_ = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    //注意sentry的构造，这样保证it指向的是第一个未到期的Timer
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);

    //Q: 这里为什么要用back_inserter_iterator来进行copy
    //A: 因为expired没有预先分配内存
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    for (Entry entry : expired)
    {
        ActiveTimer timer(entry.second, entry.second->sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1); (void)n;
    }

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp next_expire;

    //遍历超时的Timer，如果需要repeat，则重新设置时间
    //否则 删掉对应的Timer
    for (std::vector<Entry>::const_iterator it = expired.begin();
            it != expired.end(); ++it)
    {
        ActiveTimer timer(it->second, it->second->sequence());
        if (it->second->repeat()
                && canceling_timers_.find(timer) == canceling_timers_.end())
        {
            it->second->restart(now);
            insert(it->second);
        }
        else
        {
            // FIXME move to a free list
            delete it->second;
        }
    }

    //找到下一个最接近的Timer，重新设置timerfd
    if (!timers_.empty())
    {
        next_expire = timers_.begin()->second->expiration();
    }

    if (next_expire.valid())
    {
        resetTimerfd(timerfd_, next_expire);
    }
}

bool TimerQueue::insert(Timer* timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    bool earliest_changed = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();

    //如果timers为空，或者新添加的timer的timePoint小于目前最小的Timer
    //则timerfd需要重新设置
    if (it == timers_.end() || when < it->first)
    {
        earliest_changed = true;
    }

    {
        std::pair<TimerList::iterator, bool> result =
            timers_.insert(std::make_pair(when, timer));
        assert(result.second); (void)result;
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result
            = active_timers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
    }

    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}

}//namespace mouse
