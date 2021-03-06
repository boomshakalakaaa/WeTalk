#ifndef _PACKDEF_H
#define _PACKDEF_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
//#include <arpa/inet.h>
#include <ctype.h>
//#include <sys/epoll.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
//#include "err_str.h"
#include <malloc.h>
#include<QFile>
#include<iostream>
#include<map>
#include<list>


//边界值
#define _DEF_SIZE 45
#define _DEF_BUFFERSIZE 1000
#define _DEF_PORT 8002
#define _DEF_SERVERIP "192.168.231.191"  //TODO


#define TRUE true
#define FALSE false


#define _DEF_LISTEN 128
#define _DEF_EPOLLSIZE 4096
#define _DEF_IPSIZE 16
#define _DEF_COUNT 10
#define _DEF_TIMEOUT 10
#define _DEF_SQLIEN 400



#define BOOL bool
#define DEF_PACK_BASE  (10000)


////注册
//#define  DEF_PACK_REGISTER_RQ    (DEF_PACK_BASE + 0)
//#define  DEF_PACK_REGISTER_RS    (DEF_PACK_BASE + 1)
////登录
//#define  DEF_PACK_LOGIN_RQ    (DEF_PACK_BASE + 2)
//#define  DEF_PACK_LOGIN_RS    (DEF_PACK_BASE + 3)
////创建房间
//#define DEF_PACK_CREATEROOM_RQ  (DEF_PACK_BASE + 4)
//#define DEF_PACK_CREATEROOM_RS  (DEF_PACK_BASE + 5)
////加入房间
//#define DEF_PACK_JOINROOM_RQ  (DEF_PACK_BASE + 6)
//#define DEF_PACK_JOINROOM_RS  (DEF_PACK_BASE + 7)
////房间列表请求
//#define DEF_PACK_ROOM_MEMBER    (DEF_PACK_BASE + 8)
////音频数据
//#define DEF_PACK_AUDIO_FRAME    (DEF_PACK_BASE + 9)
////退出房间请求
//#define DEF_PACK_LEAVEROOM_RQ   (DEF_PACK_BASE + 10)
////退出房间回复
//#define DEF_PACK_LEAVEROOM_RS   (DEF_PACK_BASE + 11)

////强制退出
//#define DEF_PACK_FORCE_OFFLINE   ( DEF_PACK_BASE + 20 )

typedef enum Net_PACK
{
    DEF_PACK_REGISTER_RQ = 10000,
    DEF_PACK_REGISTER_RS,

    DEF_PACK_LOGIN_RQ,
    DEF_PACK_LOGIN_RS,

    DEF_PACK_CREATEROOM_RQ,
    DEF_PACK_CREATEROOM_RS,

    DEF_PACK_JOINROOM_RQ ,
    DEF_PACK_JOINROOM_RS,

    DEF_PACK_ROOM_MEMBER ,

    DEF_PACK_AUDIO_FRAME,

    DEF_PACK_LEAVEROOM_RQ ,
    DEF_PACK_LEAVEROOM_RS ,

    DEF_PACK_ADD_FRIEND_RQ,
    DEF_PACK_ADD_FRIEND_RS,

    DEF_PACK_FRIEND_INFO,

    DEF_PACK_CHAT_RQ,
    DEF_PACK_CHAT_RS,

    DEF_PACK_OFFLINRE_RQ,
    DEF_PACK_OFFLINRE_RS,

    DEF_PACK_VIDEO_FRAME,
    DEF_PACK_FORCE_OFFLINE,

    DEF_PACK_FILE_UPLOAD_RQ,
    DEF_PACK_FILE_UPLOAD_RS,
    DEF_PACK_FILEBLOCK_UPLOAD_RQ,

    DEF_PACK_DOWNLOAD_FILEINFO,
    DEF_PACK_DOWNLOAD_RQ,
    DEF_PACK_DOWNLOAD_RS,
    DEF_PACK_FILEBLOCK_DOWNLOAD_RQ,
    DEF_PACK_FILEBLOCK_DOWNLOAD_RS,

}Net_PACK;

//注册请求结果
#define userid_is_exist      0
#define register_sucess      1

//登录请求结果
#define userid_no_exist      0
#define password_error       1
#define login_sucess         2
#define user_online          3

//创建房间结果
#define room_is_exist        0
#define create_success       1

//加入房间结果
#define room_no_exist        0
#define join_success         1

//上传请求结果
#define file_is_exist        0
#define file_uploaded        1
#define file_uploadrq_sucess 2
#define file_upload_refuse   3

//上传回复结果
#define fileblock_failed     0
#define fileblock_success    1

//下载请求结果
#define file_downrq_failed   0
#define file_downrq_success  1

//添加好友结果
#define no_this_user    0
#define user_refused    1
#define user_is_offline 2
#define add_success     3
//聊天请求结果
#define normal_msg      0
#define user_offline    1
#define offline_msg     2

#define _downloadfileblock_fail  0
#define _downloadfileblock_success	1

#define DEF_PACK_COUNT (100)

#define MAX_PATH            (280 )
#define MAX_SIZE            (60  )
#define DEF_HOBBY_COUNT     (8  )
#define MAX_CONTENT_LEN     (4096 )


/////////////////////网络//////////////////////////////////////


#define DEF_MAX_BUF	  1024
#define DEF_BUFF_SIZE	  4096


typedef int PackType;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//登录请求
typedef struct STRU_LOGIN_RQ
{
    STRU_LOGIN_RQ()
    {
        m_nType = DEF_PACK_LOGIN_RQ;
        memset(m_szUser,0,MAX_SIZE);
        memset(m_szPassword,0,MAX_SIZE);
    }

    PackType m_nType;   //包类型
    char     m_szUser[MAX_SIZE] ; //用户ID
    char     m_szPassword[MAX_SIZE];  //密码
} STRU_LOGIN_RQ;


//登录回复
typedef struct STRU_LOGIN_RS
{
    STRU_LOGIN_RS()
    {
        m_nType= DEF_PACK_LOGIN_RS;
        m_UserID = 0;
        m_lResult = 0;
    }
    PackType m_nType;   //包类型
    int  m_UserID;
    int  m_lResult ; //注册结果

} STRU_LOGIN_RS;


//注册请求
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ()
    {
        m_nType = DEF_PACK_REGISTER_RQ;
        memset(m_szUser,0,MAX_SIZE);
        memset(m_szPassword,0,MAX_SIZE);
    }

    PackType m_nType;   //包类型
    char     m_szUser[MAX_SIZE] ; //用户名
    char     m_szPassword[MAX_SIZE];  //密码

} STRU_REGISTER_RQ;

//注册回复
typedef struct STRU_REGISTER_RS
{
    STRU_REGISTER_RS()
    {
        m_nType= DEF_PACK_REGISTER_RS;
        m_lResult = 0;
    }
    PackType m_nType;   //包类型
    int  m_lResult ; //注册结果

} STRU_REGISTER_RS;

//创建房间请求
typedef struct STRU_CREATEROOM_RQ
{
    STRU_CREATEROOM_RQ()
    {
        m_nType = DEF_PACK_CREATEROOM_RQ;
        m_UserID = 0;
    }

    PackType m_nType;   //包类型
    int m_UserID;       //用户ID

}STRU_CREATEROOM_RQ;

//创建房间回复
typedef struct STRU_CREATEROOM_RS
{
    STRU_CREATEROOM_RS()
    {
        m_nType= DEF_PACK_CREATEROOM_RS;
        m_lResult = 0;
        m_RoomId = 0;
    }
    PackType m_nType;   //包类型
    int  m_lResult ;    //注册结果
    int  m_RoomId;      //房间ID

}STRU_CREATEROOM_RS;

//加入房间请求
typedef struct STRU_JOINROOM_RQ
{
    STRU_JOINROOM_RQ()
    {
        m_nType = DEF_PACK_JOINROOM_RQ;
        m_UserID = 0;
        m_RoomID = 0;
    }

    PackType m_nType;   //包类型
    int m_UserID;       //用户ID
    int m_RoomID;       //房间ID

}STRU_JOINROOM_RQ;

//加入房间回复
typedef struct STRU_JOINROOM_RS
{
    STRU_JOINROOM_RS()
    {
        m_nType= DEF_PACK_JOINROOM_RS;
        m_lResult = 0;
        m_roomID = 0;
    }
    PackType m_nType;   //包类型
    int  m_lResult ;    //加入房间结果
    int m_roomID;       //房间id

}STRU_JOINROOM_RS;

//房间成员请求
typedef struct STRU_ROOM_MEMBER_RQ
{
    STRU_ROOM_MEMBER_RQ()
    {
        m_nType= DEF_PACK_ROOM_MEMBER;
        m_UserID =0;
        memset(m_szUser,0,MAX_SIZE);
    }
    PackType m_nType;           //包类型
    int m_UserID;               //用户id
    char m_szUser[MAX_SIZE];    //用户名

}STRU_ROOM_MEMBER_RQ;


//离开房间请求
typedef struct STRU_LEAVEROOM_RQ
{
    STRU_LEAVEROOM_RQ()
    {
        m_nType = DEF_PACK_LEAVEROOM_RQ;
        m_nUserId = 0;
        m_RoomId = 0;
    }
    PackType   m_nType;     //包类型
    int    m_nUserId;       //用户ID
    int    m_RoomId;        //房间ID
}STRU_LEAVEROOM_RQ;

//离开房间回复
typedef struct STRU_LEAVEROOM_RS
{
    STRU_LEAVEROOM_RS()
    {
        m_nType = DEF_PACK_LEAVEROOM_RS;
        m_UserID = 0;
        memset(szUserName,0,MAX_SIZE);
    }
    PackType   m_nType;             //包类型
    int m_UserID;                   //用户ID
    char szUserName[MAX_SIZE];      //用户名

}STRU_LEAVEROOM_RS;

//添加好友请求
typedef struct STRU_ADD_FRIEND_RQ
{
    STRU_ADD_FRIEND_RQ()
    {
        m_nType = DEF_PACK_ADD_FRIEND_RQ;
        m_userID = 0;
        memset(m_szUserName,0,MAX_SIZE);
        memset(m_szAddFriendName,0,MAX_SIZE);
    }
    PackType   m_nType;                 //包类型
    int m_userID;                       //用户ID
    char m_szUserName[MAX_SIZE];        //用户名
    char m_szAddFriendName[MAX_SIZE];   //好友名

}STRU_ADD_FRIEND_RQ;

//添加好友回复
typedef struct STRU_ADD_FRIEND_RS
{
    STRU_ADD_FRIEND_RS()
    {
        m_nType = DEF_PACK_ADD_FRIEND_RS;
        m_userID = 0;
        m_friendID = 0;
        m_result = 0;
        memset(szAddFriendName,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_userID;
    int m_friendID;
    int m_result;
    char szAddFriendName[MAX_SIZE];

}STRU_ADD_FRIEND_RS;

//好友信息
typedef struct STRU_FRIEND_INFO
{
    STRU_FRIEND_INFO()
    {
        m_nType = DEF_PACK_FRIEND_INFO;
        m_userID = 0;
        m_iconID = 0;
        m_state = 0;

        memset(m_szName,0,MAX_SIZE);
        memset(m_feeling,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_userID;
    int m_iconID;
    int m_state;
    char m_szName[MAX_SIZE];
    char m_feeling[MAX_SIZE];

}STRU_FRIEND_INFO;

//聊天请求
typedef struct STRU_CHAT_RQ
{
    STRU_CHAT_RQ()
    {
        m_nType = DEF_PACK_CHAT_RQ;
        m_result = 0;
        m_userID = 0;
        m_friendID = 0;
        memset(m_ChatContent,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_result;
    int m_userID;
    int m_friendID;
    char m_ChatContent[MAX_CONTENT_LEN];

}STRU_CHAT_RQ;

//聊天回复
typedef struct STRU_CHAT_RS
{
    STRU_CHAT_RS()
    {
        m_nType = DEF_PACK_CHAT_RS;
    }
    PackType   m_nType;   //包类型
    int m_userID;
    int m_friendID;
    int m_result;

}STRU_CHAT_RS;

//离线请求
typedef struct STRU_OFFLINE_RQ
{
    STRU_OFFLINE_RQ()
    {
        m_nType = DEF_PACK_OFFLINRE_RQ;
    }
    PackType   m_nType;   //包类型
    int m_userID;

}STRU_OFFLINE_RQ;

//离线回复
typedef struct STRU_OFFLINE_RS
{
    STRU_OFFLINE_RS()
    {
        m_nType = DEF_PACK_OFFLINRE_RS;
    }
    PackType   m_nType;   //包类型
    int m_userID;

}STRU_OFFLINE_RS;

//强制下线
typedef struct STRU_FORCE_OFFLINE
{
    STRU_FORCE_OFFLINE()
    {
        m_nType = DEF_PACK_FORCE_OFFLINE;
        m_UserID = 0;
    }
    PackType   m_nType;   //包类型
    int m_UserID;

}STRU_FORCE_OFFLINE;

//好友信息
typedef struct STRU_USER_INFO
{
    STRU_USER_INFO(){
        id= 0;
        fd= 0;
        nState = 0;
        memset(szUserName , 0 , MAX_SIZE);
        memset(feeling , 0 , MAX_SIZE);
        iconID = 0;
    }
    int id;
    int fd;
    char szUserName[MAX_SIZE];
    int nState;
    int  iconID;
    char feeling[MAX_SIZE];

}UserInfo;
//=============================================文件=============================================
//上传文件请求
typedef struct STRU_UPLOAD_RQ
{
    STRU_UPLOAD_RQ()
    {
        m_nType = DEF_PACK_FILE_UPLOAD_RQ;
        m_senderId = 0;
        m_recverId = 0;
        m_nFileId = 0;
        m_nFileSize = 0;
        memset(m_szFileName , 0 ,MAX_PATH);
        memset(m_szFileType , 0 ,MAX_SIZE);
    }
    PackType m_nType;                   //包类型
    int      m_senderId;                //发送者id
    int      m_recverId;                //接收者id
    int      m_nFileId;                 //文件id
    int64_t  m_nFileSize;               //文件大小
    char     m_szFileName[MAX_PATH];    //文件名
    char     m_szFileType[MAX_SIZE];    //文件类型
}STRU_UPLOAD_RQ;

//上传文件成功回复
typedef struct STRU_UPLOAD_RS
{
    STRU_UPLOAD_RS()
    {
        m_nType = DEF_PACK_FILE_UPLOAD_RS;
        m_nResult = 0;
    }
    PackType m_nType;   //包类型
    int      m_nResult;

}STRU_UPLOAD_RS;

//上传文件块请求
typedef struct STRU_UPLOAD_FILEBLOCK_RQ
{
    STRU_UPLOAD_FILEBLOCK_RQ()
    {
        m_nType = DEF_PACK_FILEBLOCK_UPLOAD_RQ;
        m_nUserId = 0;
        m_nFileId =0;
        m_nBlockLen =0;
        ZeroMemory(m_szFileContent,MAX_CONTENT_LEN);
    }
    PackType m_nType;                   //包类型
    int    m_nUserId;                   //用户ID
    int    m_nFileId;
    int    m_nBlockLen;
    char   m_szFileContent[MAX_CONTENT_LEN];

}STRU_UPLOAD_FILEBLOCK_RQ;

typedef struct STRU_FILEINFO
{
    int m_nFileId;                  //用来区分是发给哪个个控件的  上传的时候 是一个随机数
    int m_nUserId;                  //发送者id
    int m_nRecverId;
    int64_t m_nPos;                 //文件偏移位置
    int64_t m_nFilesize;            //文件大小
    QFile *pFile;                    //文件指针
    char m_szFileName[MAX_PATH];    //文件名
    char m_szFilePath[MAX_PATH];    //文件路径
    char m_szFileType[MAX_SIZE];

}FileInfo;

//下载文件请求头
typedef struct STRU_DOWNLOAD_FILEINFO
{
    STRU_DOWNLOAD_FILEINFO()
    {
        m_nType = DEF_PACK_DOWNLOAD_FILEINFO;
        m_nSenderId = 0;
        m_nRecverId = 0;
        m_nFileSize =0;
        memset(m_szFileName , 0 ,MAX_PATH);
        memset(m_szFileType , 0 ,MAX_SIZE);
    }
    PackType m_nType;                   //包类型
    int      m_nSenderId;               //发送者ID
    int      m_nRecverId;               //接收者ID
    int64_t  m_nFileSize;               //文件大小
    char     m_szFileName[MAX_PATH];    //文件名
    char     m_szFileType[MAX_SIZE];    //文件类型

}STRU_DOWNLOAD_FILEINFO;

//下载文件请求

typedef struct STRU_DOWNLOAD_RQ
{
    STRU_DOWNLOAD_RQ()
    {
        m_nType = DEF_PACK_DOWNLOAD_RQ;
        m_nUserId = 0;
    }
    PackType   m_nType;   //包类型
    int    m_nUserId; //用户ID

}STRU_DOWNLOAD_RQ;

//下载文件回复
typedef struct STRU_DOWNLOAD_RS
{
    STRU_DOWNLOAD_RS()
    {
        m_nType = DEF_PACK_DOWNLOAD_RS;
        m_nFileId = 0;
        m_nFileSize = 0;
        memset(m_szFileName , 0 ,MAX_PATH);
        memset(m_szFileType , 0 ,MAX_SIZE);
    }
    PackType m_nType;   //包类型
    int      m_nFileId;
    int64_t  m_nFileSize;
    char     m_szFileName[MAX_PATH];
    char     m_szFileType[MAX_SIZE];    //文件类型
}STRU_DOWNLOAD_RS;

//下载文件块请求
typedef struct STRU_DOWNLOAD_FILEBLOCK_RQ
{
    STRU_DOWNLOAD_FILEBLOCK_RQ()
    {
        m_nType = DEF_PACK_FILEBLOCK_DOWNLOAD_RQ;
        m_nUserId = 0;
        m_nFileId = 0;
        m_nBlockLen =0;
        memset(m_szFileContent , 0 ,MAX_CONTENT_LEN);
    }
    PackType m_nType;   //包类型
    int    m_nUserId; //用户ID
    int    m_nFileId;
    int    m_nBlockLen;
    char   m_szFileContent[MAX_CONTENT_LEN];
}STRU_DOWNLOAD_FILEBLOCK_RQ;

//下载文件块回复包
typedef struct STRU_DOWNLOAD_FILEBLOCK_RS
{
    STRU_DOWNLOAD_FILEBLOCK_RS()
    {
        m_nType = DEF_PACK_FILEBLOCK_DOWNLOAD_RS;
        m_nUserId = 0;
        m_nFileId =0;
        m_nBlockLen =0;
        m_nResult = 0;
    }
    PackType m_nType;   //包类型
    int    m_nUserId; //用户ID
    int    m_nFileId;
    int    m_nResult;
    int    m_nBlockLen;
}STRU_DOWNLOAD_FILEBLOCK_RS;


////下载文件请求
//typedef struct STRU_DOWNLOAD_RQ
//{
//    STRU_DOWNLOAD_RQ()
//    {
//        m_nType = DEF_PACK_FILE_DOWNLOAD_RQ;
//        m_nUserId = 0;
//    }
//    PackType   m_nType;   //包类型
//    int    m_nUserId; //用户ID

//}STRU_DOWNLOAD_RQ;

////下载文件回复
//typedef struct STRU_DOWNLOAD_RS
//{
//    STRU_DOWNLOAD_RS()
//    {
//        m_nType = DEF_PACK_FILE_DOWNLOAD_RS;
//        m_nFileId = 0;
//        memset(m_szFileName , 0 ,MAX_PATH);
//        memset(m_rtmp , 0 ,MAX_PATH);
//    }
//    PackType m_nType;   //包类型
//    int      m_nFileId;
//    int64_t  m_nFileSize;
//    int      m_nVideoId;
//    char     m_szFileName[MAX_PATH];
//    char     m_rtmp[MAX_PATH];

//}STRU_DOWNLOAD_RS;

////下载文件块请求

//typedef struct STRU_DOWNLOAD_FILEBLOCK_RQ
//{
//    STRU_DOWNLOAD_FILEBLOCK_RQ()
//    {
//        m_nType = DEF_PACK_FILEBLOCK_DOWNLOAD_RQ;
//        m_nUserId = 0;
//        m_nFileId =0;
//        m_nBlockLen =0;
//        ZeroMemory(m_szFileContent,MAX_CONTENT_LEN);
//    }
//    PackType m_nType;   //包类型
//    int    m_nUserId; //用户ID
//    int    m_nFileId;
//    int    m_nBlockLen;
//    char     m_szFileContent[MAX_CONTENT_LEN];
//}STRU_DOWNLOAD_FILEBLOCK_RQ;


////下载文件块回复包
//typedef struct STRU_DOWNLOAD_FILEBLOCK_RS
//{
//    STRU_DOWNLOAD_FILEBLOCK_RS()
//    {
//        m_nType = DEF_PACK_FILEBLOCK_DOWNLOAD_RQ;
//        m_nUserId = 0;
//        m_nFileId =0;
//        m_nBlockLen =0;
//        m_nResult = 0;
//    }
//    PackType m_nType;   //包类型
//    int    m_nUserId; //用户ID
//    int    m_nFileId;
//    int     m_nResult;
//    INT64    m_nBlockLen;
//}STRU_DOWNLOAD_FILEBLOCK_RS;


#endif



