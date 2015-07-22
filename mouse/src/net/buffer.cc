#include "buffer.h"
#include "sockets_ops.h"

#include <glog/logging.h>

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

using namespace mouse;

ssize_t Buffer::readFd(int fd, int* saved_errno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const ssize_t n = readv(fd, vec, 2);
    if (n < 0)
    {
        *saved_errno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        writer_index_ += n;
    }
    else
    {
        writer_index_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

