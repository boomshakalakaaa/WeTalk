#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H



#include "TCPNet.h"
#include "Mysql.h"

class TcpKernel;
//成员函数指针，要想调用普通的类成员函数 必须声明为成员函数指针
typedef void (TcpKernel::*PFUN)(int,char*,int nlen);

typedef struct
{
    PackType m_type;
    PFUN m_pfun;
} ProtocolMap;



class TcpKernel:public IKernel
{
public:
    int Open();
    void Close();
    void DealData(int,char*,int);

    //注册
    void RegisterRq(int,char*,int);
    //登录
    void LoginRq(int,char*,int);
    void AddFriendRq(int,char*,int);
    void AddFriendRs(int,char*,int);
    void ChatRq(int,char*,int);
    void CreateRoomRq(int,char*,int);
    void JoinRoomRq(int,char*,int);
    void LeaveRoomRq(int,char*,int);
    void AudioFrameRq(int,char*,int);
    void VideoFrameRq(int,char*,int);
    void UploadRq(int clientfd, char *szbuf, int nlen);
    void UploadFileBlockRq(int clientfd, char *szbuf, int nlen);
    void DownloadRq(int clientfd, char *szbuf, int nlen);
    void DownloadFileBlockRs(int clientfd, char *szbuf, int nlen);

public:
    UserInfo* getUserInfoFromSql(int id);
    void sendUserList(int id);
    void sendMsgToOnlineClient(int id,char* szbuf,int nlen);
    void InitRandomNum();
    void getOfflineMsg(int id);

private:
    CMysql * m_sql;
    TcpNet * m_tcp;

    map<int,UserInfo*> m_mapIDToUserInfo;
    map<int,list<UserInfo*>> m_mapRoomIDToUserList;
    list<FileInfo*> m_fileList;
    list<FileInfo*> m_fileDownLoadList;
};

#endif
