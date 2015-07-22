#ifndef MOUSE_NET_EVENTLOOP_H
#define MOUSE_NET_EVENTLOOP_H

#include "../base/timestamp.h"
#include "callbacks.h"
#include "timer_id.h"

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace mouse
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop
{
    //nocopyable
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void startLoop();

    void quit();

    // Time when poll returns, usually means data arrivial.
    Timestamp pollReturnTime() const { return poll_return_time_; }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(const Functor& cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(const Functor& cb);

    // Runs callback at 'time'.
    TimerId runAt(const Timestamp& time, const TimerCallback& cb);

    // Runs callback after @c delay seconds.
    TimerId runAfter(double delay, const TimerCallback& cb);

    // Runs callback every @c interval seconds.
    TimerId runEvery(double interval, const TimerCallback& cb);

    void cancel(TimerId timer_id);

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread()
    {
        return thread_id_ == std::this_thread::get_id();
    }

private:
    void abortNotInLoopThread();
    //for Wakeup, implement by eventfd
    void handleRead();
    void doPendingFunctors();

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    bool calling_pending_functors_;
    Timestamp poll_return_time_;
    const std::thread::id thread_id_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    ChannelList active_channels_;
    //for wakeup
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;
    std::mutex mutex_;
    std::vector<Functor> pending_functors_; // @GuardedBy mutex_
};

}//namespace mouse

#endif
