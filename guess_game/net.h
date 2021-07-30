#ifndef __NET_H__

#include <time.h>
#include <functional>

#define NET_NONE 0           //未设置
#define NET_READABLE 1       //事件可读
#define NET_WRITABLE 2       //事件可写

class EventLoop;
class Poller;

/* Types and data structures */
// 回调函数
typedef std::function<void()> EventCallback;

class NetEvent
{
public:
    NetEvent(int fd);
    ~NetEvent();
public:
    void handleEvent();
    void setRevents(int revt) { revents_ = revt; }
    int fd() const { return fd_; }
    int events() const { return events_; }
    void enableReading() { events_ |= NET_READABLE; }
    void disableReading() { events_ &= ~NET_READABLE; }
    void enableWriting() { events_ |= NET_WRITABLE; }
    void disableWriting() { events_ &= ~NET_WRITABLE; }
    void disableAll() { events_ = NET_NONE; }
private:
    int events_;
    int revents_;
    const int fd_;
    EventCallback readCallback_;
    EventCallback writeCallback_;
};

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();
public:
    void loop();
private:
    bool quit_;
    Poller* poller_;
    NetEvent** active_events_; //激活事件
};

#endif //__NET_H__