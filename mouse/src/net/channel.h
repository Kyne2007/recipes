#ifndef MOUSE_NET_CHANNEL_H
#define MOUSE_NET_CHANNEL_H

#include "../base/timestamp.h"

#include <functional>

namespace mouse
{

class EventLoop;

class Channel
{
    //nocopyable
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receive_time);
    void setReadCallback(const ReadEventCallback& cb) { read_callback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { write_callback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { error_callback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { close_callback_ = cb; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revent) { revents_ = revent; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    bool isWriting() const { return events_ & kWriteEvent; }

    //for Poller
    int index() { return index_; }
    void setIndex(int idx) { index_ = idx; }

    EventLoop* loop() { return loop_; }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int  fd_;
    int        events_;
    int        revents_;
    int        index_;

    bool event_handling_;

    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
    EventCallback close_callback_;
};

}//namespace mouse

#endif
