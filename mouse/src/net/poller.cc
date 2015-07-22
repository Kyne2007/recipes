#include "poller.h"

#include "channel.h"

#include <glog/logging.h>

#include <assert.h>
#include <poll.h>

using namespace mouse;

Poller::Poller(EventLoop* loop)
    : owner_loop_(loop)
{
}

Poller::~Poller()
{
}

Timestamp Poller::poll(int timeout_ms, ChannelList* active_channels)
{
    int num_events = ::poll(pollfds_.data(), pollfds_.size(), timeout_ms);
    Timestamp now(Timestamp::now());

    if (num_events > 0)
    {
        DLOG(INFO) << num_events << " events happened";
        fillActiveChannels(num_events, active_channels);
    }
    else if (num_events == 0)
    {
        DLOG(INFO) << "nothing happened";
    }
    else
    {
        LOG(ERROR) << "Poller::poll()";
    }

    return now;
}

void Poller::fillActiveChannels(int num_events, ChannelList* active_channels) const
{
    for (PollFdList::const_iterator pfd = pollfds_.begin();
            pfd != pollfds_.end() && num_events > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --num_events;

            ChannelMap::const_iterator it_channel = channels_.find(pfd->fd);
            assert(it_channel != channels_.end());
            Channel* channel = it_channel->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            active_channels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();

    DLOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->index() < 0)
    {
        //a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());

        struct pollfd fd;
        fd.fd = channel->fd();
        fd.events = static_cast<short>(channel->events());
        fd.revents = 0;
        pollfds_.push_back(fd);

        int index = static_cast<int>(pollfds_.size()) - 1;
        channel->setIndex(index);
        channels_[fd.fd] = channel;
    }
    else
    {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);

        int index = channel->index();
        assert(0 <= index && index < static_cast<int>(pollfds_.size()));
        struct pollfd& fd = pollfds_[index];
        assert(fd.fd == channel->fd() || fd.fd == -channel->fd() - 1);
        fd.events = static_cast<short>(channel->events());
        fd.revents = 0;
        fd.fd = channel->fd();
        if (channel->isNoneEvent()) {
            // ignore this pollfd
            fd.fd = -channel->fd() - 1;
        }
    }
}

void Poller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    DLOG(INFO) << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());

    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1); (void)n;
    if (static_cast<size_t>(idx) == pollfds_.size() - 1)
    {
        pollfds_.pop_back();
    }
    else
    {
        int channel_at_end = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channel_at_end < 0)
        {
            channel_at_end = -channel_at_end - 1;
        }
        channels_[channel_at_end]->setIndex(idx);
        pollfds_.pop_back();
    }
}

