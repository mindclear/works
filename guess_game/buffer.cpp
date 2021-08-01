#include "buffer.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

static const int INIT_BUFFER_BYTES = 1024;

Buffer::Buffer()
    :read_pos_(0), write_pos_(0), capacity_(INIT_BUFFER_BYTES)
{
    buffer_ = (char*)calloc(1, sizeof(char*) * capacity_);
}

Buffer::~Buffer()
{
    free(buffer_);
}

void Buffer::ensureWritableBytes(int nlen)
{
    int wbytes = writableBytes();
    if (wbytes >= nlen)
        return;

    if (wbytes + read_pos_ >= nlen)
    {
        int rbytes = readableBytes();
        memcpy(buffer_, buffer_ + read_pos_, rbytes);
        read_pos_ = 0;
        write_pos_ = rbytes;
    }
    else
    {
        //FIXME:调整大小
        buffer_ = (char*)realloc(buffer_, write_pos_ + nlen);
        capacity_ = write_pos_ + nlen;
    }
    assert(writableBytes() >= nlen);
}

void Buffer::append(const char *s, int nlen)
{
    ensureWritableBytes(nlen);
    memcpy(buffer_ + write_pos_, s, nlen);
    write_pos_ += nlen;
}

void Buffer::retrieve(int nlen)
{
    assert(nlen <= readableBytes());
    if (nlen < readableBytes())
    {
      read_pos_ += nlen;
    }
    else
    {
        read_pos_ = 0;
        write_pos_ = 0;
    }
}

ssize_t Buffer::readFd(int fd)
{
    int nread = read(fd, peek(), writableBytes());
    if (nread > 0)
        write_pos_ += nread;
    return nread;
}