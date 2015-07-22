#ifndef MOUSE_NET_POLLER_H
#define MOUSE_NET_POLLER_H

#include <map>
#include <vector>

#include "../base/timestamp.h"
#include "event_loop.h"

struct pollfd;

namespace mouse
{

class Channel;

class Poller
{
    //nocopyable
    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    ~Poller();

    Timestamp poll(int timeout_ms, ChannelList* active_channels);

    void updateChannel(Channel* channel);

    /// Remove the channel, when it destructs.
    /// Must be called in the loop thread.
    void removeChannel(Channel* channel);


    void assertInLoopThread() { owner_loop_->assertInLoopThread(); }

private:
    void fillActiveChannels(int num_events, ChannelList* active_channels) const;

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* owner_loop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

}//namespace mouse

#endif
