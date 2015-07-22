#ifndef MOUSE_NET_BUFFER_H
#define MOUSE_NET_BUFFER_H

#include <algorithm>
#include <string>
#include <vector>

#include <assert.h>

namespace mouse
{

class Buffer
{

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer()
        : buffer_(kCheapPrepend + kInitialSize),
          reader_index_(kCheapPrepend),
          writer_index_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    // default copy-ctor, dtor and assignment are fine

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(reader_index_, rhs.reader_index_);
        std::swap(writer_index_, rhs.writer_index_);
    }

    size_t readableBytes() const
    {
        return writer_index_ - reader_index_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writer_index_;
    }

    size_t prependableBytes() const
    {
        return reader_index_;
    }

    const char* peek() const
    {
        return begin() + reader_index_;
    }

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        reader_index_ += len;
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveAll()
    {
        reader_index_ = kCheapPrepend;
        writer_index_ = kCheapPrepend;
    }

    std::string retrieveAsString()
    {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }

    void append(const std::string& str)
    {
        append(str.data(), str.length());
    }

    void append(const char* /*restrict*/ data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const void* /*restrict*/ data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite()
    {
        return begin() + writer_index_;
    }

    const char* beginWrite() const
    {
        return begin() + writer_index_;
    }

    void hasWritten(size_t len)
    {
        writer_index_ += len;
    }

    void prepend(const void* /*restrict*/ data, size_t len)
    {
        assert(len <= prependableBytes());
        reader_index_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + reader_index_);
    }

    void shrink(size_t reserve)
    {
        std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
        std::copy(peek(), peek() + readableBytes(), buf.begin() + kCheapPrepend);
        buf.swap(buffer_);
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t readFd(int fd, int* saved_errno);

private:
    char* begin()
    {
        return buffer_.data();
    }

    const char* begin() const
    {
        return buffer_.data();
    }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writer_index_+len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < reader_index_);
            size_t readable = readableBytes();
            std::copy(begin() + reader_index_,
                    begin() + writer_index_,
                    begin() + kCheapPrepend);
            reader_index_ = kCheapPrepend;
            writer_index_ = reader_index_ + readable;
            assert(readable == readableBytes());
        }
    }

    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;
};

} //namespace Mouse

#endif
