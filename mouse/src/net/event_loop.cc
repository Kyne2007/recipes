#include "event_loop.h"

#include "channel.h"
#include "poller.h"
#include "timer_queue.h"

#include <glog/logging.h>

#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/eventfd.h>

using namespace mouse;

__thread EventLoop* t_loop_in_this_thread = 0;
const int kPollTimeMs = 10000;

static int createEventfd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        LOG(ERROR) << "Failed in eventfd";
        abort();
    }
    return fd;
}

//Ignore SIGPIPE
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe initObj;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      calling_pending_functors_(false),
      thread_id_(std::this_thread::get_id()),
      poller_(new Poller(this)),
      timer_queue_(new TimerQueue(this)),
      wakeup_fd_(createEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_))
{
    LOG(INFO) << "EventLoop created " << this << " in thread " << thread_id_;
    if (t_loop_in_this_thread)
    {
        LOG(FATAL) << "Another EventLoop " << t_loop_in_this_thread
                   << " exists in this thread " << thread_id_;
    }
    else
    {
        t_loop_in_this_thread = this;
    }

    wakeup_channel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeup_channel_->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    ::close(wakeup_fd_);
    t_loop_in_this_thread = NULL;
}

void EventLoop::startLoop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        active_channels_.clear();
        poll_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);
        for (ChannelList::iterator it = active_channels_.begin();
                it != active_channels_.end(); it++)
        {
            (*it)->handleEvent(poll_return_time_);
        }

        doPendingFunctors();
    }

    LOG(INFO) << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_.push_back(cb);
    }

    if (!isInLoopThread() || calling_pending_functors_)
    {
        wakeup();
    }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
    return timer_queue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    //int64_t us = interval * (duration_cast<microseconds>(seconds(1))).count();
    //TimePoint time = Clock::now() + microseconds(us);
    return timer_queue_->addTimer(cb, time, interval);
}

void EventLoop::cancel(TimerId timer_id)
{
  return timer_queue_->cancel(timer_id);
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->loop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->loop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in thread_id_ = " << thread_id_
            << ", current thread id = " <<  std::this_thread::get_id();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        //other thread may modify pending_functors_
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }

    calling_pending_functors_ = false;
}

