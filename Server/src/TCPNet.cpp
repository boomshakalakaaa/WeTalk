#include <TCPNet.h>
#include <TCPKernel.h>

TcpNet * TcpNet::m_pThis = 0;
//初始化网络
int TcpNet::InitNetWork()
{
    //线程池初始化
    pool_t *pool = NULL;
    m_pool = new thread_pool;
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    if(inet_pton(AF_INET,_DEF_SERVERIP,&serveraddr.sin_addr.s_addr) == -1)
    {
        perror("Init Ip Error:");
        return FALSE;
    }
    serveraddr.sin_port = htons(_DEF_PORT);
    //创建Socket
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        perror("Create Socket Error:");
        return FALSE;
    }
    int mw_optval;
    //设置 端口复用，TIME_WAIT的端口可以直接拿来使用，避免服务器重启时，需要更改端口号的情况
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)&mw_optval,sizeof(mw_optval));
    //绑定端口号
    if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) == -1)
    {
        perror("Bind Socket Error:");
        return FALSE;
    }
    //监听socket
    if(listen(sockfd,_DEF_LISTEN) == -1)
    {
        perror("Listen Error:");
        return FALSE;
    }
    //创建epoll的句柄，可以监听的文件描述符为_DEF_EPOLLSIZE，参数在2.6版本的linux内核后，只要大于0都没有影响
    epfd = epoll_create(_DEF_EPOLLSIZE);
    //将监听socket添加到红黑树上
    Addfd(sockfd,TRUE,false);
    //创建拥有10个线程的线程池 最大线程数200 环形队列最大值50
    if((pool = (m_pool->Pool_create(200,10,50))) == NULL)
        err_str("Create Thread_Pool Failed:",-1);

    //主线程监听epoll_wait等待链接到来
    m_pool->Producer_add(pool, EPOLL_Jobs, pool);
    return TRUE;
}

//向内核事件表添加节点
void TcpNet::Addfd(int fd,int enable_et/*是否为边缘触发*/,int enable_oneshot)
{
    struct epoll_event eptemp;
    eptemp.events = EPOLLIN;
    eptemp.data.fd = fd;
    if(enable_et)
        eptemp.events |= EPOLLET;
    if(enable_oneshot)
        eptemp.events |= EPOLLONESHOT;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&eptemp);
}
void TcpNet::Deletefd(int fd)
{
    epoll_ctl(epfd,EPOLL_CTL_DEL,fd,0);
//    struct epoll_event eptemp;
//    eptemp.data.fd = fd;
//    eptemp.events = EPOLLET | EPOLLIN | EPOLLONESHOT;
//    epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&eptemp);
}
//调用epoll_wait等待就绪事件到来
void * TcpNet::EPOLL_Jobs(void * arg)
{
    pool_t *pool = (pool_t*)arg;
    int ready;
    int i = 0;
    while(1)
    {
        printf("%d\n",i++);
        printf("EPOLL_Jobs thread[%ld]\n",pthread_self());
        //阻塞-1监听socket 当监听到了事件时，
        if((ready = epoll_wait(m_pThis->epfd,m_pThis->epollarr,_DEF_EPOLLSIZE,-1)) == -1)
            err_str("Epoll Call Failed:",-1);//出错
        m_pThis->Epoll_Deal(ready,pool);
        bzero(m_pThis->epollarr,sizeof(epollarr));
    }
}
//从就绪队列取就绪事件到用户空间，进行链接处理和信息接收
void TcpNet::Epoll_Deal(int ready,pool_t *pool)
{
    printf("Epoll_Deal thread[%ld]\n",pthread_self());
    int i = 0;
    for(i=0; i<ready; i++)
    {
        int fd = epollarr[i].data.fd;
        if(sockfd == fd)   //客户端建立链接
            m_pool->Producer_add(pool,Accept_Deal,NULL);
        else if(epollarr[i].events & EPOLLIN)
        {
            Deletefd(fd);
            m_pool->Producer_add(pool,Info_Recv,(void*)fd);
        }
    }
}
//链接处理
void *TcpNet::Accept_Deal(void *arg)
{

    struct sockaddr_in clientaddr;
    int clientsize = sizeof(clientaddr);
    int clientfd;
    char ipstr[_DEF_IPSIZE];
    pthread_mutex_lock(&m_pThis->alock);
    printf("Accept_Deal thread[%ld]\n",pthread_self());
    if((clientfd = accept(m_pThis->sockfd,(struct sockaddr*)&clientaddr,(socklen_t*)&clientsize)) == -1)
    {
        err_str("Custom Thread Accept Error",-1);
    }
    pthread_mutex_unlock(&m_pThis->alock);
    m_pThis->Addfd(clientfd,TRUE,false);
    printf("Custom Thread TID:0x%x\tClient IP:%s\tClient PORT:%d\t\n",(unsigned int)pthread_self()
           ,inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,ipstr,sizeof(ipstr)),ntohs(clientaddr.sin_port));
    return 0;
}
//接收信息 递交中介者处理
void *TcpNet::Info_Recv(void *arg)
{
    printf("Info_Recv thread[%ld]\n",pthread_self());
    int clientfd = (long)arg;
    int nRelReadNum = 0;
    int nPackSize = 0;
    char *pSzBuf = NULL;
    nRelReadNum = recv(clientfd,&nPackSize,sizeof(nPackSize),0);
    if(nRelReadNum <= 0)
    {
        close(clientfd);
        return NULL;
    }
    pSzBuf = (char*)malloc(sizeof(char)*nPackSize);
    int nOffSet = 0;
    nRelReadNum = 0;
    //接收包的数据
    while(nPackSize)
    {
        nRelReadNum = recv(clientfd,pSzBuf+nOffSet,nPackSize,0);
        if(nRelReadNum > 0)
        {
            nOffSet += nRelReadNum;
            nPackSize -= nRelReadNum;
        }
    }
    m_pThis->m_kernel->DealData(clientfd,pSzBuf,nOffSet);
    //m_pThis->Deletefd(clientfd);
    m_pThis->Addfd(clientfd,TRUE,FALSE);
    printf("pszbuf = %p \n",pSzBuf);
    if(pSzBuf != NULL)
    {
        free(pSzBuf);
        pSzBuf = NULL;
    }
    return 0;
}

int TcpNet::SendData(int clientfd,char* szbuf,int nlen)
{
    //先发包大小 再发包内容
    if(send(clientfd,(const char *)&nlen,sizeof(int),0)<0)
        return FALSE;
    if(send(clientfd,szbuf,nlen,0)<0)
        return FALSE;
    return TRUE;
}

void TcpNet::UnInitNetWork()
{
    close(sockfd);
}
