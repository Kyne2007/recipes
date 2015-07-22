#ifndef MOUES_NET_EVENT_LOOP_THREAD_H
#define MOUSE_NET_EVENT_LOOP_THREAD_H

#include <condition_variable>
#include <mutex>
#include <thread>

namespace mouse
{

class EventLoop;

class EventLoopThread
{
    //nocopyable
    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(const EventLoopThread&) = delete;

public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    std::unique_ptr<std::thread> thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

};

} //namespace Mouse

#endif
