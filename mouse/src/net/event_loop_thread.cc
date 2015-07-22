#include "event_loop_thread.h"

#include "event_loop.h"

#include <assert.h>

using namespace mouse;

EventLoopThread::EventLoopThread()
    : loop_(NULL),
      exiting_(false),
      thread_(nullptr),
      mutex_(),
      cond_()
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    loop_->quit();
    thread_->join();
}

EventLoop* EventLoopThread::startLoop()
{
    assert(thread_ == nullptr);
    thread_.reset(new std::thread(std::bind(&EventLoopThread::threadFunc, this)));

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait(lock);
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.startLoop();
    //assert(exiting_);
}

