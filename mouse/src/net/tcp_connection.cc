#include "tcp_connection.h"

#include "channel.h"
#include "event_loop.h"
#include "socket.h"
#include "sockets_ops.h"

#include <glog/logging.h>

#include <functional>

#include <assert.h>
#include <errno.h>
#include <stdio.h>

using namespace mouse;

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& local_address,
                             const InetAddress& peer_address) 
    : loop_(CHECK_NOTNULL(loop)),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      local_address_(local_address),
      peer_address_(peer_address)
{
    DLOG(INFO) << "TcpConnection::ctor[" <<  name_ << "] at " << this
        << " fd=" << sockfd;
    using std::placeholders::_1;
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
    DLOG(INFO) << "TcpConnection::dtor[" <<  name_ << "] at " << this
        << " fd=" << channel_->fd();
}

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            //message will be copy
            loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && output_buffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0)
        {
            if (static_cast<size_t>(nwrote) < message.size())
            {
                DLOG(INFO) << "I am going to write more data";
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG(ERROR) << "TcpConnection::SendInLoop";
            }
        }
    }

    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < message.size())
    {
        output_buffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    // FIXME: use compare and swap
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();
    connection_callback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    connection_callback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(Timestamp receive_time)
{
    int saved_errno = 0;
    ssize_t n = input_buffer_.readFd(channel_->fd(), &saved_errno);
    if (n > 0)
    {
        message_callback_(shared_from_this(), &input_buffer_, receive_time);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saved_errno;
        LOG(ERROR) << "TcpConnection::handleRead";
        handleClose();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(),
                            output_buffer_.peek(),
                            output_buffer_.readableBytes());
        if (n > 0)
        {
            output_buffer_.retrieve(n);
            //数据写完了就关闭连接
            //如果想要长连接呢？
            if (output_buffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                //ShutdownInLoop()会判断当前连接是否还有未写数据
                //写完了之后才会关闭连接
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
            else
            {
                DLOG(INFO) << "I am going to write more data";
            }
        }
        else
        {
            LOG(ERROR) << "TcpConnection::handleWrite";
        }
    }
    else
    {
        DLOG(INFO) << "Connection is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    DLOG(INFO) << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    channel_->disableAll();
    // must be the last line
    close_callback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG(ERROR) << "TcpConnection::handleError [" << name_
        << "] - SO_ERROR = " << err << " " << strerror(err);
}

