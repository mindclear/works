#ifndef __POLLER_H__
#define __POLLER_H__

#include <sys/epoll.h>

class NetEvent;

class Poller
{
public:
    Poller();
    virtual ~Poller();
public:
    virtual bool updateNetEvent(NetEvent* event) = 0;
    virtual bool delNetEvent(NetEvent* event) = 0;
    virtual int poll(NetEvent** active_events, int ac_num, int timeout_ms) = 0;
protected:
    NetEvent** register_events_; //所有注册事件，FIXME：use map
};

class EPoller : public Poller
{
public:
    EPoller();
    ~EPoller();
public:
    virtual bool updateNetEvent(NetEvent* event);
    virtual bool delNetEvent(NetEvent* event);
    virtual int poll(NetEvent** active_events, int ac_num, int timeout_ms);
private:
    int epollfd_;
    epoll_event* epoll_events_; // 临时保存就绪事件
};
#endif //__POLLER_H__