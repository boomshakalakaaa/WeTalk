#ifndef _TCPNET_H
#define _TCPNET_H

#include "Thread_pool.h"
#include "packdef.h"
class IKernel
{
public:
    virtual void DealData(int,char*,int) = 0;
};

class TcpNet
{
public:
    //构造 使用父类Ikernel指针指向子类TCPKernel对象
    TcpNet(IKernel* kernel){ m_kernel = kernel; }
    int InitNetWork();
    void UnInitNetWork();
    int SendData(int,char*,int);
    void Addfd(int, int, int enable_oneshot);
    void Deletefd(int);
    void Epoll_Deal(int,pool_t*);
public:
    //线程函数
    static void *EPOLL_Jobs(void*);
    static void *Accept_Deal(void*);
    static void *Info_Recv(void*);
    //线程函数中使用的静态this指针 由于 线程函数传递参数需要为pool
    void SetpThis(TcpNet * tcp){ m_pThis = tcp ; }
    static TcpNet * m_pThis;
public:
    //读事件用户空间数组
    struct epoll_event epollarr[_DEF_EPOLLSIZE];
    //监听socket
    int sockfd;
    //用户socket
    int clientfd;
    //网络地址参数
    struct sockaddr_in serveraddr;
    //内核事件表
    int epfd;
    //中介者
    IKernel * m_kernel;
    //线程池
    thread_pool * m_pool;
    //读写锁
    pthread_mutex_t alock;
    pthread_mutex_t rlock;

};

#endif
