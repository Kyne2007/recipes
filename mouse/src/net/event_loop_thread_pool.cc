#include "event_loop_thread_pool.h"

#include "event_loop.h"
#include "event_loop_thread.h"

#include <assert.h>
#include <functional>
#include <utility>

using namespace mouse;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop)
    : base_loop_(base_loop),
      started_(false),
      threads_num_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start()
{
    assert(!started_);
    base_loop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < threads_num_; ++i) {
        EventLoopThread* t = new EventLoopThread;
        threads_.push_back(std::move(std::unique_ptr<EventLoopThread>(t)));
        loops_.push_back(t->startLoop());
    }
}

//如果threads_num_等于0的话，则base_loop也给TcpConnection使用
EventLoop* EventLoopThreadPool::getNextLoop()
{
    base_loop_->assertInLoopThread();
    EventLoop* loop = base_loop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

