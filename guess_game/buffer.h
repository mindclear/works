#ifndef __BUFFER_H__

#include <stdint.h>
#include <unistd.h>

//FIXME:动态调整大小
class Buffer
{
public:
    Buffer();
    ~Buffer();
    void ensureWritableBytes(int nlen);
    void append(const char *s, int nlen);
    void retrieve(int nlen);
    char* peek() { return buffer_ + read_pos_; }
    // char* peekWrite() { return buffer_ + write_pos_; }
    int readableBytes() { return write_pos_ - read_pos_;}
    int writableBytes() { return capacity_ - write_pos_;}
    ssize_t readFd(int fd);
private:
    int read_pos_;
    int write_pos_;
    int capacity_;
    char* buffer_;
};

#endif //__BUFFER_H__