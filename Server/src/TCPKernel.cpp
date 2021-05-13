#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>
#include <sys/time.h>

using namespace std;


static const ProtocolMap m_ProtocolMapEntries[] =
{
    {DEF_PACK_REGISTER_RQ , &TcpKernel::RegisterRq},
    {DEF_PACK_LOGIN_RQ , &TcpKernel::LoginRq},
    {DEF_PACK_ADD_FRIEND_RQ , &TcpKernel::AddFriendRq},
    {DEF_PACK_ADD_FRIEND_RS , &TcpKernel::AddFriendRs},
    {DEF_PACK_CHAT_RQ , &TcpKernel::ChatRq},
    {DEF_PACK_CREATEROOM_RQ , &TcpKernel::CreateRoomRq},
    {DEF_PACK_JOINROOM_RQ , &TcpKernel::JoinRoomRq},
    {DEF_PACK_LEAVEROOM_RQ , &TcpKernel::LeaveRoomRq},
    {DEF_PACK_AUDIO_FRAME , &TcpKernel::AudioFrameRq},
    {DEF_PACK_VIDEO_FRAME , &TcpKernel::VideoFrameRq},
    {DEF_PACK_FILE_UPLOAD_RQ, &TcpKernel::UploadRq},
    {DEF_PACK_FILEBLOCK_UPLOAD_RQ, &TcpKernel::UploadFileBlockRq},
    {DEF_PACK_DOWNLOAD_RQ,&TcpKernel::DownloadRq},
    {DEF_PACK_DOWNLOAD_RQ,&TcpKernel::DownloadFileBlockRs},
    {0,0}
};
#define RootPath   "/home/htg123/File/"

//初始化 各模块
int TcpKernel::Open()
{
    //数据库、网络模块的初始化
    InitRandomNum();
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_tcp->SetpThis(m_tcp);
    pthread_mutex_init(&m_tcp->alock,NULL);
    pthread_mutex_init(&m_tcp->rlock,NULL);
    if(  !m_sql->ConnectMysql("localhost","root","111111","WeTalk")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    if( !m_tcp->InitNetWork()  )
    {
        printf("InitNetWork Failed...\n");
        return FALSE;
    }
    else
    {
        printf("Init Net Success...\n");
    }

    return TRUE;
}

void TcpKernel::Close()
{
    for(auto ite = m_mapIDToUserInfo.begin();ite != m_mapIDToUserInfo.end();ite++)
    {
        delete ite->second;
    }

    m_mapIDToUserInfo.clear();

    m_sql->DisConnect();
    m_tcp->UnInitNetWork();
}


void TcpKernel:: DealData(int clientfd,char *szbuf,int nlen)
{
    PackType *pType = (PackType*)szbuf;
    int i = 0;
    while(1)
    {
        if(*pType == m_ProtocolMapEntries[i].m_type)
        {
            auto fun= m_ProtocolMapEntries[i].m_pfun;
            (this->*fun)(clientfd,szbuf,nlen);
        }
        else if(m_ProtocolMapEntries[i].m_type == 0 && m_ProtocolMapEntries[i].m_pfun == 0)
            return;
        ++i;
    }
    return;
}

////注册请求结果
//#define userid_is_exist      0
//#define register_sucess      1

////登录请求结果
//#define userid_no_exist      0
//#define password_error       1
//#define login_sucess         2
//#define user_online          3

//注册
void TcpKernel::RegisterRq(int clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d RegisterRq\n", clientfd);

    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;
    //查表 查找用户名
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select name from t_user where name= '%s';",rq->m_szUser);
    list<string> resList;
    bool res = m_sql->SelectMysql(sqlBuf,1,resList);
    if(!res)
    {
        cout<<"SelectMysql error:"<<sqlBuf<<endl;
        return ;
    }
    if(resList.size() > 0)
    {
        //有 不成功
        rs.m_lResult = userid_is_exist;
        cout<<"userid_is_exist"<<endl;
    }else
    {
        //没有 成功
        char sqlBuf[_DEF_SQLIEN] = "";
        sprintf(sqlBuf,"insert into t_user (name,password) values('%s','%s');",
                rq->m_szUser,rq->m_szPassword);
        m_sql->UpdataMysql(sqlBuf);

        //通过用户名查询id
        sprintf(sqlBuf,"select id from t_user where name = '%s'",rq->m_szUser);
        list<string> resID;
        m_sql->SelectMysql(sqlBuf,1,resID);
        int id = 0;
        if(resID.size()>0)
        {
            id = atoi(resID.front().c_str());

        }
        //插入用户信息
        sprintf(sqlBuf,"insert into t_userinfo (id,name,icon,feeling) values(%d, '%s', %d, '%s');",
                id,rq->m_szUser,0,"");
        m_sql->UpdataMysql(sqlBuf);

        rs.m_lResult = register_sucess;
        cout<<"register_sucess"<<endl;
    }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}
//登录
void TcpKernel::LoginRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;
    //查表 查找用户名
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select password,id from t_user where name='%s';",rq->m_szUser);
    list<string> resList;
    bool res = m_sql->SelectMysql(sqlBuf,2,resList);
    if(!res)
    {
        cout<<"SelectMysql error:"<<sqlBuf;
        return ;
    }
    //查找到用户
    if(resList.size() > 0)
    {
        //对比密码
        if(strcmp( resList.front().c_str() , rq->m_szPassword ) == 0)
        {
            //密码正确 登录成功
            rs.m_lResult = login_sucess;
            resList.pop_front();
            rs.m_UserID = atoi(resList.front().c_str());

            if(m_mapIDToUserInfo.find(rs.m_UserID) == m_mapIDToUserInfo.end())
            {
                //没找到，新建
                UserInfo* info = new UserInfo;
                info->id = rs.m_UserID;
                info->fd = clientfd;
                strcpy(info->szUserName,rq->m_szUser);
                info->nState = 1;
                //根据映射关系<id,Userinfo*>，存储映射
                m_mapIDToUserInfo[rs.m_UserID] = info;

            }else
            {
                //找到了 发强制下线包
                STRU_FORCE_OFFLINE off;
                off.m_UserID = rs.m_UserID;
                UserInfo* info = m_mapIDToUserInfo[rs.m_UserID];
                m_tcp->SendData( info->fd , (char*)&off , sizeof(off) );
                //改info
                info->fd = clientfd;
                //存map
                m_mapIDToUserInfo[rs.m_UserID] = info;
            }
            //显示好友列表 发送上线通知
            getUserInfoFromSql(rs.m_UserID);
            sendUserList(rs.m_UserID);
            //TODO:去数据库拉取离线消息
            getOfflineMsg(rs.m_UserID);

        }else
        {
            //密码错误 登录失败
            rs.m_lResult = password_error;

        }
    }else
    {
        rs.m_lResult = userid_no_exist;
    }


    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}
//添加好友申请
void TcpKernel::AddFriendRq(int clientfd, char *szbuf, int nlen)
{
//    //添加好友结果
//    #define no_this_user    0
//    #define user_refused    1
//    #define user_is_offline 2
//    #define add_success     3
    printf("clientfd:%d AddFriendRq\n", clientfd);
    //解析包
    STRU_ADD_FRIEND_RQ* rq = (STRU_ADD_FRIEND_RQ*)szbuf;
    STRU_ADD_FRIEND_RS rs;
    rs.m_userID = rq->m_userID;
    strcpy(rs.szAddFriendName,rq->m_szAddFriendName);

    //根据用户名查数据库，找对应id
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select id from t_user where name='%s'",rq->m_szAddFriendName);
    list<string> resID;
    m_sql->SelectMysql(sqlBuf,1,resID);
    if(resID.size() > 0)
    {
        //找到了对应id 根据id看是否在线
        int id = atoi(resID.front().c_str());

        if(m_mapIDToUserInfo.find(id)!=m_mapIDToUserInfo.end())
        {
            //在线，转发包
            this->sendMsgToOnlineClient(id,szbuf,nlen);
            return ;
        }else
        {
            //不在线，返回不在线结果
            rs.m_result = user_is_offline;
        }

    }else
    {
        //没有找到对应id，返回结果 查无此人
        rs.m_result = no_this_user;

    }
    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

}
//添加好友回复
void TcpKernel::AddFriendRs(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d AddFriendRs\n", clientfd);
    //解析包
    STRU_ADD_FRIEND_RS* rs = (STRU_ADD_FRIEND_RS*)szbuf;
    //查看结果
    if(rs->m_result == add_success)
    {
        //成功，更新数据库（双向添加好友）
        char sqlBuf[_DEF_SQLIEN] = "";
        sprintf(sqlBuf,"insert into t_friend (idA,idB) values(%d,%d)",rs->m_userID,rs->m_friendID);
        m_sql->UpdataMysql(sqlBuf);

        sprintf(sqlBuf,"insert into t_friend (idA,idB) values(%d,%d)",rs->m_friendID,rs->m_userID);
        m_sql->UpdataMysql(sqlBuf);
        //发好友列表
        this->getUserInfoFromSql(rs->m_userID);
        this->sendUserList(rs->m_userID);

    }
    //转发
    this->sendMsgToOnlineClient(rs->m_userID,szbuf,nlen);
}
//处理聊天请求包
void TcpKernel::ChatRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d ChatRq\n", clientfd);
    //解包
    STRU_CHAT_RQ* rq = (STRU_CHAT_RQ*) szbuf;

    //查 好友是否在线
    if(m_mapIDToUserInfo.find(rq->m_friendID) != m_mapIDToUserInfo.end())
    {
        //在线，直接转发包
        this->sendMsgToOnlineClient(rq->m_friendID,szbuf,nlen);
    }else
    {
        //未在线，封装未在线回复包
        STRU_CHAT_RS rs;
        rs.m_result = user_is_offline;
        rs.m_userID = rq->m_userID;
        rs.m_friendID = rq->m_friendID;
        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        //TODO:将离线消息存放到数据库
        char sqlBuf[_DEF_SQLIEN] = "";
        sprintf(sqlBuf,"insert into t_offlineChat (idA,idB,content) values(%d,%d,'%s');",rq->m_userID,rq->m_friendID,rq->m_ChatContent);
        if(m_sql->UpdataMysql(sqlBuf) == TRUE)
        {
            printf("insert mysql succeed\n");
        }else
            printf("insert mysql failed\n");
    }
}
//时间随机房间id
void TcpKernel::InitRandomNum()
{
    struct timeval time;
    gettimeofday(&time,NULL);
    srand(time.tv_sec+time.tv_usec);
}
//创建房间请求
void TcpKernel::CreateRoomRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d CreateRoomRq\n", clientfd);
    //解析包
    STRU_CREATEROOM_RQ* rq = (STRU_CREATEROOM_RQ*) szbuf;

    //通过id判断是否在线，如果不在线，不管
    if(m_mapIDToUserInfo.find(rq->m_UserID) == m_mapIDToUserInfo.end())
        return;
    UserInfo *info = m_mapIDToUserInfo[rq->m_UserID];

    //roomid 随机产生-->如果 0 重复 重新随机
    int roomid = 0;
    do
    {
        roomid = rand()%10000;

    }while(roomid == 0 || (m_mapRoomIDToUserList.find(roomid) != m_mapRoomIDToUserList.end()));
    //将创建者添加到链表中
    list<UserInfo*> lst;
    lst.push_back(info);
    //存储到map中
    m_mapRoomIDToUserList[roomid] = lst;
    //回复包
    STRU_CREATEROOM_RS rs;
    rs.m_RoomId = roomid;
    rs.m_lResult = 1;
    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

}
//加入房间请求
void TcpKernel::JoinRoomRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d JoinRoomRq\n", clientfd);
    STRU_JOINROOM_RQ* rq = (STRU_JOINROOM_RQ*)szbuf;
    STRU_JOINROOM_RS rs;

    //通过用户id查找信息，未找到，发送加入房间失败回复包
    if(m_mapIDToUserInfo.find(rq->m_UserID) == m_mapIDToUserInfo.end())
    {
        rs.m_lResult = 0;
        rs.m_roomID = 0;
        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        return;
    }
    UserInfo* joiner = m_mapIDToUserInfo[rq->m_UserID];
    //通过roomid，查找是否有该房间，未找到，发加入房间失败回复包
    if(m_mapRoomIDToUserList.find(rq->m_RoomID) == m_mapRoomIDToUserList.end())
    {
        rs.m_lResult = 0;
        rs.m_roomID = 0;
        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        return;
    }
    //房间成员信息包
    STRU_ROOM_MEMBER_RQ joinRq;
    joinRq.m_UserID = joiner->id;
    strcpy(joinRq.m_szUser, joiner->szUserName);
    //拿到list，发送加入房间成功回复包
    list<UserInfo*> lst = m_mapRoomIDToUserList[rq->m_RoomID];
    rs.m_lResult = 1;
    rs.m_roomID = rq->m_RoomID;
    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

    //遍历链表
    for(auto ite = lst.begin();ite != lst.end();ite++)
    {
        UserInfo* inner = *ite;
        STRU_ROOM_MEMBER_RQ innerRq;
        innerRq.m_UserID = inner->id;
        strcpy(innerRq.m_szUser,inner->szUserName);
        //发送给加入者，房间内其他人的信息
        m_tcp->SendData(joiner->fd,(char*)&innerRq,sizeof(innerRq));
        //发送给房间内其他人，加入者的信息
        m_tcp->SendData(inner->fd,(char*)&joinRq,sizeof(joinRq));
    }

    //将加入者添加到list
    lst.push_back(joiner);
    //更新map
    m_mapRoomIDToUserList[rq->m_RoomID] = lst;
}
//离开房间请求
void TcpKernel::LeaveRoomRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d LeaveRoomRq\n", clientfd);
    //解包
    STRU_LEAVEROOM_RQ* rq = (STRU_LEAVEROOM_RQ*)szbuf;
    //找用户信息，看是否在线,未在线，不作处理
    if(m_mapIDToUserInfo.find(rq->m_nUserId) == m_mapIDToUserInfo.end())  return;
    //存储用户信息
    UserInfo* leaver = m_mapIDToUserInfo[rq->m_nUserId];
    //找房间信息，没有房间，不作处理
    if(m_mapRoomIDToUserList.find(rq->m_RoomId) == m_mapRoomIDToUserList.end())  return;
    //存储房间用户信息链表
    list<UserInfo*> lst = m_mapRoomIDToUserList[rq->m_RoomId];
    //封包
    STRU_LEAVEROOM_RS rs;
    rs.m_UserID = leaver->id;
    strcpy(rs.szUserName,leaver->szUserName);
    //遍历链表
    auto ite = lst.begin();
    while(ite != lst.end())
    {
        //通过id找离开者
        if((*ite)->id == leaver->id)
        {
            //移除离开者的信息
            ite = lst.erase(ite);
        }
        else
        {
            //发包给  房间中其他人  通知  离开者 离开房间
            this->sendMsgToOnlineClient((*ite)->id,(char*)&rs,sizeof(rs));
            ++ite;
        }
    }
    //更新map
    m_mapRoomIDToUserList[rq->m_RoomId] = lst;


}
//音频帧请求
void TcpKernel::AudioFrameRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d AudioFrameRq\n", clientfd);
    //反序列化
    char* tmp = szbuf;
    tmp += sizeof(int);
    int userID = *(int*)tmp;
    tmp += sizeof(int);
    int roomID = *(int*)tmp;
    tmp += sizeof(int);
    //音频帧
    if(m_mapRoomIDToUserList.find(roomID) != m_mapRoomIDToUserList.end())
    {
        list<UserInfo*> lst = m_mapRoomIDToUserList[roomID];
        //遍历链表
        for(auto ite = lst.begin();ite != lst.end();++ite)
        {
            UserInfo* user = *ite;
            if(user->id != userID)
            {
                this->sendMsgToOnlineClient(user->id,szbuf,nlen);
            }
        }
    }

}
//视频帧请求
void TcpKernel::VideoFrameRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d VideoFrameRq\n", clientfd);
    //反序列化
    char* tmp = szbuf;
    tmp += sizeof(int);
    int userID = *(int*)tmp;
    tmp += sizeof(int);
    int roomID = *(int*)tmp;
    tmp += sizeof(int);
    //视频帧
    if(m_mapRoomIDToUserList.find(roomID) != m_mapRoomIDToUserList.end())
    {
        list<UserInfo*> lst = m_mapRoomIDToUserList[roomID];
        //遍历链表
        for(auto ite = lst.begin();ite != lst.end();++ite)
        {
            UserInfo* user = *ite;
            if(user->id != userID)
            {
                this->sendMsgToOnlineClient(user->id,szbuf,nlen);
            }
        }
    }
}

//传输文件请求
void TcpKernel::UploadRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d UploadRq\n", clientfd);
    //拆包
    STRU_UPLOAD_RQ* rq = (STRU_UPLOAD_RQ*)szbuf;
    //读文件信息 准备存储到链表中
    FileInfo *info = (FileInfo*)malloc(sizeof(FileInfo));
    info->m_nFileId = rq->m_nFileId;
    info->m_nUserId = rq->m_senderId;
    info->m_nRecverId = rq->m_recverId;
    info->m_nPos = 0;
    info->m_nFilesize = rq->m_nFileSize;
    memcpy(info->m_szFileName,rq->m_szFileName,MAX_PATH);
    memcpy(info->m_szFileType,rq->m_szFileType,MAX_SIZE);
    //根据id查用户名
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select name from t_user where id=%d;",rq->m_senderId);
    list<string> nameList;
    m_sql->SelectMysql(sqlBuf,1,nameList);
    //如果查到用户名，拼接保存路径
    if(nameList.size() > 0)
    {
        const char* szUserName = nameList.front().c_str();
        sprintf( info->m_UserName,"%s",szUserName);
        sprintf( info->m_szFilePath ,"%s%s",RootPath,szUserName);
    }else
    {
        err_str("SelectMySql Falied:",-1);
        free( info );
        info = NULL;
        return;
    }
    //查看目录是否存在，如果不存在，创建目录
    if(access(info->m_szFilePath,0) == -1)
    {
        if(mkdir(info->m_szFilePath,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)==-1)
        {
            cout<<"1"<<endl;
        }

    }
    //拼接保存路径
    char m_szFilePath[MAX_PATH];
    strcpy(m_szFilePath,info->m_szFilePath);
    sprintf( m_szFilePath ,"%s%s/%s",RootPath,nameList.front().c_str(),rq->m_szFileName);
    //通过路径打开文件
    info->pFile = fopen(m_szFilePath,"w");
    if(info->pFile)
    {
        //打开成功，加入链表
        m_fileList.push_back(info);
    }else
    {
        free(info);
        info = NULL;
    }
}
void TcpKernel::UploadFileBlockRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d UploadFileBlockRq\n", clientfd);
    //拆包
    STRU_UPLOAD_FILEBLOCK_RQ* rq = (STRU_UPLOAD_FILEBLOCK_RQ*)szbuf;
    //文件信息、实际写入的长度
    FileInfo* info = nullptr;
    int64_t nLen = 0;

    //遍历数据包链表
    auto ite = m_fileList.begin();
    while(ite != m_fileList.end())
    {
        //根据文件id、发送者id查找当前应处理的包，找到 赋值给info，跳出
        if(rq->m_nFileId == (*ite)->m_nFileId && rq->m_nUserId == (*ite)->m_nUserId)
        {
            info = (*ite);
            //写入文件内容
            if(info)
            {
                //待写入的内容、待写入的每个元素的大小（字节）、待写入元素个数、指向写入的文件的指针
                nLen = fwrite(rq->m_szFileContent,1,rq->m_nBlockLen,info->pFile);
                //pos向后偏移
                info->m_nPos += nLen;
                //当偏移量大于等于文件大小文件写完
                if(info->m_nPos >= info->m_nFilesize)
                {
                    //关闭文件指针
                    fclose(info->pFile);

                    //存储到数据库
                    char sqlBuf[_DEF_SQLIEN] = "";
                    sprintf(sqlBuf,"insert into t_file (fileid,sender,receiver,file_path,file_name,file_type,file_size) values(%d,%d,%d,'%s','%s','%s',%ld);",
                            info->m_nFileId,info->m_nUserId,info->m_nRecverId,info->m_szFilePath,info->m_szFileName,info->m_szFileType,info->m_nFilesize);
                    m_sql->UpdataMysql(sqlBuf);

                    //写下载文件的信息包
                    STRU_DOWNLOAD_FILEINFO file;
                    file.m_nFileSize = info->m_nFilesize;
                    file.m_nSenderId = info->m_nUserId;
                    memcpy(file.m_szFileName,info->m_szFileName,MAX_PATH);
                    memcpy(file.m_szFileType,info->m_szFileType,MAX_SIZE);
                    if(m_mapIDToUserInfo.find(info->m_nRecverId) != m_mapIDToUserInfo.end())
                    {
                        this->sendMsgToOnlineClient(info->m_nRecverId,(char*)&file,sizeof(file));
                    }


                    //写上传回复包
                    STRU_UPLOAD_RS rs;
                    rs.m_nResult = 1;
                    //删除info结点信息
                    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
                    ite = m_fileList.erase(ite);                   
                    free(info);
                    info = nullptr;
                    break;
                }
            }
        }
        ++ite;
    }
}
//处理客户端下载文件请求
void TcpKernel::DownloadRq(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d DownloadRq\n", clientfd);
    //拆包
    STRU_DOWNLOAD_RQ* rq = (STRU_DOWNLOAD_RQ*)szbuf;
    //根据userid 到 数据库查文件路径
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select fileid,file_path,file_name,file_type,file_size from t_file where receiver=%d",rq->m_nUserId);

    list<string> res;
    m_sql->SelectMysql(sqlBuf,5,res);

    FileInfo* info = new FileInfo;
    if(res.size() > 0)
    {
        //查到文件id 存储到fileid中
        info->m_nFileId = atoi(res.front().c_str());
        res.pop_front();
        //查到路径拷贝到 数组中
        strcpy(info->m_szFilePath,res.front().c_str());
        res.pop_front();
        //查到名字拷贝到 数组中
        strcpy(info->m_szFileName,res.front().c_str());
        res.pop_front();
        //查到类型拷贝到数组中
        strcpy(info->m_szFileType,res.front().c_str());
        res.pop_front();
        //查到文件大小存到file_size中
        info->m_nFilesize = atoi(res.front().c_str());
        res.pop_front();
    }
    info->m_nPos = 0;
    //拼接打开文件的路径
    strcat(info->m_szFilePath,"/");
    strcat(info->m_szFilePath,info->m_szFileName);
    //通过路径打开文件描述符

    FILE* pFile = fopen(info->m_szFilePath , "r");
    if( pFile )
    {
        STRU_DOWNLOAD_RS rs;
        rs.m_nFileId = info->m_nFileId;
        rs.m_nFileSize = info->m_nFilesize;
        strcpy( rs.m_szFileName , info->m_szFileName);
        strcpy( rs.m_szFileType , info->m_szFileType);
        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

        m_fileDownLoadList.push_back(info);

        while(1)
        {
            //文件内容 内容长度 文件Id,文件位置--传输请求
            STRU_DOWNLOAD_FILEBLOCK_RQ blockrq;
            int64_t nRelReadNum = (int64_t)fread(blockrq.m_szFileContent,1,MAX_CONTENT_LEN,pFile);

            blockrq.m_nBlockLen = nRelReadNum;
            blockrq.m_nFileId = info->m_nFileId;
            blockrq.m_nUserId = rq->m_nUserId;

            printf("nRelReadNum:%d\n",blockrq.m_nBlockLen);

            m_tcp->SendData(clientfd,(char*)&blockrq,sizeof(blockrq));

            info->m_nPos += nRelReadNum;
            if(info->m_nPos == info->m_nFilesize)
            {
                fclose(pFile);
                free( info );
                char sqlBuf[_DEF_SQLIEN] = "";
                //从数据库中将离线文件信息删除
                sprintf(sqlBuf,"delete from t_file where receiver=%d;",info->m_nUserId);
                if(m_sql->UpdataMysql(sqlBuf) == TRUE)
                {
                    printf("aaaaaaa\n");
                }
                break;
            }
        }
    }
}
void TcpKernel::DownloadFileBlockRs(int clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d DownloadFileBlockRs\n", clientfd);
    //拆包
    STRU_DOWNLOAD_FILEBLOCK_RS *rs = (STRU_DOWNLOAD_FILEBLOCK_RS *)szbuf;
    //获取info文件信息
    auto ite = m_fileDownLoadList.begin();
    FileInfo* info = nullptr;
    while(ite != m_fileDownLoadList.end())
    {
        if(rs->m_nFileId == (*ite)->m_nFileId && rs->m_nUserId == (*ite)->m_nUserId)
        {
            info = *ite;
            if(info)
            {
                //继续发送
                STRU_DOWNLOAD_FILEBLOCK_RQ rq;
                rq.m_nFileId = info->m_nFileId;
                rq.m_nUserId = info->m_nUserId;
                //如果文件块客户端接收失败，再次发送
                if(rs->m_nResult == _downloadfileblock_fail)
                {
                    //移动文件指针pInfo->m_nDownLoadSize
                    fseeko64(info->pFile,info->m_nPos,SEEK_SET);
                    int64_t nRelReadNum = (int64_t)fread(rq.m_szFileContent,1,MAX_CONTENT_LEN,info->pFile);
                    rq.m_nBlockLen = nRelReadNum;
                    //读文件内容并发送
                    m_tcp->SendData(clientfd,(char*)&rq,sizeof(rq));
                    return;
                }
                info->m_nPos += rs->m_nBlockLen;

                if(info->m_nPos == info->m_nFilesize)
                {
                    //关闭文件
                    fclose(info->pFile);
                    m_fileDownLoadList.erase(ite);
                    free( info );

                    return;
                }
                else
                {
                    int64_t nRelReadNum = (int64_t)fread(rq.m_szFileContent,1,MAX_CONTENT_LEN,info->pFile);
                    rq.m_nBlockLen = nRelReadNum;
                    printf("m_nBlockLen:%d\n",rq.m_nBlockLen);
                }
                m_tcp->SendData(clientfd,(char*)&rq,sizeof(rq));
            }
        }
        ++ite;
    }
}
//从数据库获取用户信息
UserInfo* TcpKernel::getUserInfoFromSql(int id)
{
    if(m_mapIDToUserInfo.find(id) == m_mapIDToUserInfo.end())   return NULL;

    UserInfo* info = m_mapIDToUserInfo[id];
    //从数据库中取 并 更新信息
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select name,icon,feeling from t_userinfo where id=%d",id);
    list<string> res;
    m_sql->SelectMysql(sqlBuf,3,res);
    if(res.size() > 0)
    {
        strcpy(info->szUserName,res.front().c_str());
        res.pop_front();
        info->iconID = atoi(res.front().c_str());
        res.pop_front();
        strcpy(info->feeling,res.front().c_str());
        res.pop_front();
    }
    return info;
}
//将id = id的用户的好友列表 信息发送给 用户 并把这个人的 信息再发给好友
void TcpKernel::sendUserList(int id)
{
    //找该用户，没找到直接返回
    if(m_mapIDToUserInfo.find(id) == m_mapIDToUserInfo.end()) return;

    //写用户信息loginer 用户请求loginRq
    UserInfo* loginerInfo = m_mapIDToUserInfo[id];
    STRU_FRIEND_INFO  loginRq;
    loginRq.m_state = 1;
    loginRq.m_iconID = loginerInfo->iconID;
    strcpy(loginRq.m_szName,loginerInfo->szUserName);
    strcpy(loginRq.m_feeling,loginerInfo->feeling);
    loginRq.m_userID = loginerInfo->id;

    //查找与idA关联的所有好友
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select idB from t_friend where idA = %d",id);
    list<string> listID;
    m_sql->SelectMysql(sqlBuf,1,listID);
    if(listID.size() == 0) return ;

    //遍历得到该用户所有的好友id
    //查找所有好友列表 friender 用户信息请求 frienderRq
    //好友信息发给登录的人，登录人的信息发给每个好友
    for(auto ite = listID.begin();ite != listID.end();++ite)
    {
        int friendid = atoi(ite->c_str());
        STRU_FRIEND_INFO friendRq;
        friendRq.m_userID = friendid;
        //查找好友idB的是否在线
        if(m_mapIDToUserInfo.find(friendid) != m_mapIDToUserInfo.end())
        {
            //好友在线，发送 登录用户的 信息
            friendRq.m_state = 1;
            //先从缓存好友的取信息存到包里
            UserInfo* friendInfo = m_mapIDToUserInfo[friendid];
            friendRq.m_iconID = friendInfo->iconID;
            strcpy(friendRq.m_szName,friendInfo->szUserName);
            strcpy(friendRq.m_feeling,friendInfo->feeling);

            //向好友发送登录者信息
            m_tcp->SendData( friendInfo->fd , (char*)&loginRq , sizeof(loginRq) );
        }
        else
        {
            //好友不在线，从数据库取信息
            friendRq.m_state = 0;
            //从数据库中取 并 更新信息
            char sqlBuf[_DEF_SQLIEN] = "";
            sprintf(sqlBuf,"select name,icon,feeling from t_userinfo where id=%d",friendid);
            list<string> res;
            m_sql->SelectMysql(sqlBuf,3,res);
            if(res.size() > 0)
            {
                strcpy(friendRq.m_szName,res.front().c_str());
                res.pop_front();
                friendRq.m_iconID = atoi(res.front().c_str());
                res.pop_front();
                strcpy(friendRq.m_feeling,res.front().c_str());
                res.pop_front();
            }
        }
        //向登录的人发送好友信息
        m_tcp->SendData( loginerInfo->fd , (char*)&friendRq , sizeof(friendRq) );
    }

}

//转发给在线用户
void TcpKernel::sendMsgToOnlineClient(int id,char* szbuf,int nlen)
{
    if(m_mapIDToUserInfo.find(id) != m_mapIDToUserInfo.end())
    {
        m_tcp->SendData(m_mapIDToUserInfo[id]->fd,szbuf,nlen);
    }
}

void TcpKernel::getOfflineMsg(int id)
{   
    //获取离线消息
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select idA,content from t_offlineChat where idB=%d;",id);
    list<string> resList;
    bool res = m_sql->SelectMysql(sqlBuf,2,resList);
    if(!res)
    {
        cout<<"SelectMysql error:"<<sqlBuf;
        return ;
    }

    //查找到用户
    while(resList.size() > 0)
    {
        //给用户发送离线消息
        STRU_CHAT_RQ rq;
        rq.m_result = offline_msg;
        rq.m_userID = id;
        rq.m_friendID = atoi(resList.front().c_str());
        resList.pop_front();
        strcpy(rq.m_ChatContent,resList.front().c_str());
        resList.pop_front();

        sendMsgToOnlineClient(id,(char*)&rq,sizeof(rq));

    }
    //从数据库中将离线消息删除
    sprintf(sqlBuf,"delete from t_offlineChat where idB=%d;",id);
    m_sql->UpdataMysql(sqlBuf);
    //获取离线文件信息
    sprintf(sqlBuf,"select sender,file_name,file_type,file_size from t_file where receiver=%d;",id);
    //写下载文件的信息包
    STRU_DOWNLOAD_FILEINFO file;
    res = m_sql->SelectMysql(sqlBuf,4,resList);
    if(!res)
    {
        cout<<"SelectMysql error:"<<sqlBuf;
        return ;
    }
    while(resList.size() > 0)
    {
        file.m_nSenderId = atoi(resList.front().c_str());
        resList.pop_front();
        strcpy(file.m_szFileName,resList.front().c_str());
        resList.pop_front();
        strcpy(file.m_szFileType,resList.front().c_str());
        resList.pop_front();
        file.m_nFileSize = atoi(resList.front().c_str());
        resList.pop_front();
        sendMsgToOnlineClient(id,(char*)&file,sizeof(file));
    }


}
