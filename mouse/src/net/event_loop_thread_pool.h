#ifndef MOUSE_NET_EVENT_LOOP_THREAD_POOL_H
#define MOUSE_NET_EVENT_LOOP_THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace mouse
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
    //nocopyable
    EventLoopThreadPool(const EventLoopThreadPool&) = delete;
    EventLoopThreadPool& operator=(const EventLoopThread&) = delete;

public:
    EventLoopThreadPool(EventLoop* base_loop);
    ~EventLoopThreadPool();
    void setThreadsNum(int threads_num) { threads_num_ = threads_num; }
    void start();
    EventLoop* getNextLoop();


private:
    EventLoop* base_loop_;
    bool started_;
    int threads_num_;
    int next_;  // always in loop thread
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}//namespace mouse

#endif
