#ifndef __NET_H__
#define __NET_H__

#include <time.h>
#include <functional>
#include <memory>
#include "buffer.h"

#define NET_NONE 0           //未设置
#define NET_READABLE 1       //事件可读
#define NET_WRITABLE 2       //事件可写

class EventLoop;
class Poller;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// 回调函数
class TcpConnection;
typedef std::function<void()> EventCallback;
typedef std::function<void()> Functor;
typedef std::function<void (TcpConnection*)> ConnectionCallback;
typedef std::function<void (TcpConnection*, Buffer*)> MessageCallback;
typedef std::function<void (TcpConnection*)> CloseCallback;

//事件
class NetEvent
{
public:
    NetEvent(int fd);
    ~NetEvent();
public:
    void handleEvent();
    void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setRevents(int revt) { revents_ = revt; }
    int fd() const { return fd_; }
    int events() const { return events_; }
    void enableReading() { events_ |= NET_READABLE; }
    void disableReading() { events_ &= ~NET_READABLE; }
    void enableWriting() { events_ |= NET_WRITABLE; }
    void disableWriting() { events_ &= ~NET_WRITABLE; }
    void disableAll() { events_ = NET_NONE; }
    bool isWriting() { return events_ & NET_WRITABLE; }
private:
    int events_;    //关注事件
    int revents_;   //就绪事件
    const int fd_;  //文件描述符
    EventCallback readCallback_;    //读回调函数
    EventCallback writeCallback_;   //写回调函数
};

struct FunctorNode
{
    Functor functor;    //函数体
    FunctorNode* next;

    FunctorNode()
        :next(NULL)
    {}
};

//事件循环
class EventLoop
{
public:
    EventLoop();
    ~EventLoop();
public:
    void loop();
    void updateNetEvent(NetEvent* event);
    void queueInLoop(Functor cb);
private:
    void doFunctors();
private:
    bool quit_;
    Poller* poller_;
    NetEvent** active_events_; //激活事件，FIXME：use vector
    //解决tcp connection生命周期
    //FIXME:use vector
    FunctorNode* head_functor_;
    FunctorNode* tail_functor_;
};

//连接对象
//FIXME:使用share_ptr保证TcpConnection的生命周期
class TcpConnection
{
public:
    TcpConnection(EventLoop* loop, int fd);
    ~TcpConnection();
    void send(const void* data, int nlen);
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    void setContext(void* context) { context_ = context; }
    void* getContext() const { return context_; }
    bool connected() const { return connect_; }
private:
    void handleRead();
    void handleClose();
    void handleWrite();
    
private:
    EventLoop* loop_;
    int fd_;
    bool connect_;
    NetEvent* event_;       //事件
    Buffer input_buffer_;   //读缓冲区
    Buffer output_buffer_;  //写缓冲区
    MessageCallback messageCallback_;       //消息回调
    ConnectionCallback connectionCallback_; //连接回调
    CloseCallback closeCallback_; //关闭回调
    void* context_; //保存应用数据
};

//监听SERVER
class TcpServer
{
public:
    TcpServer(EventLoop* loop, uint16_t port);
    ~TcpServer();
    bool start();
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
private:
    int createTcpSocket(uint16_t port);
    void newConnection(int connfd, const char* peer_ip, const uint16_t peer_port);
    void handleAccept();
    void removeConnection(TcpConnection* conn);
private:
    int listen_fd_;
    uint16_t listen_port_;  //监听端口
    EventLoop* loop_;       //循环loop
    NetEvent* listen_event_;
    MessageCallback messageCallback_;       //消息回调
    ConnectionCallback connectionCallback_; //连接回调
};

#endif //__NET_H__