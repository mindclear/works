#ifndef __POLLER_H__

#include <sys/epoll.h>

class NetEvent;

class Poller
{
public:
    Poller(int max_events);
    virtual ~Poller();
public:
    virtual bool addNetEvent(NetEvent* event) = 0;
    virtual bool delNetEvent(NetEvent* event) = 0;
    virtual int poll(NetEvent** active_events, int ac_num, int timeout_ms) = 0;
protected:
    int max_events_;
    NetEvent** register_events_; //所有注册事件，FIXME：MAP
};

class EPoller : public Poller
{
public:
    EPoller(int max_events);
    ~EPoller();
public:
    virtual bool addNetEvent(NetEvent* event);
    virtual bool delNetEvent(NetEvent* event);
    virtual int poll(NetEvent** active_events, int ac_num, int timeout_ms);
private:
    void update(int operation, NetEvent* event);
private:
    int epollfd_;
    int event_num_;
    epoll_event* epoll_events_; // 事件表
};
#endif //__POLLER_H__