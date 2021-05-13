#ifndef WETALK_H
#define WETALK_H

#include <QWidget>
#include <logindialog.h>
#include <qmytcpclient.h>
#include <IMToolBox.h>
#include "chatdialog.h"
#include <useritem.h>
#include <QMap>
#include <QList>
#include <QCloseEvent>
#include "Packdef.h"
#include <QMenu>
#include "addfriend.h"
#include "roomdialog.h"
#include "videoitem.h"
#include "video_read.h"
#include "audio_read.h"
#include "audio_write.h"
#include "fileitem.h"
class ChatDialog;
namespace Ui {
class WeTalk;
}

class WeTalk : public QWidget
{
    Q_OBJECT

public:
    explicit WeTalk(QWidget *parent = 0);
    ~WeTalk();
private slots:
    void on_tb_friend_clicked();
    void on_tb_group_clicked();
    void on_tb_zone_clicked();
    void on_pb_close_clicked();
    void closeEvent(QCloseEvent *event);

    void on_pb_menu_clicked();

public slots:
    void slot_loginCommit(QString name,QString password);       //登录处理槽函数
    void slot_registerCommit(QString name,QString password);    //注册处理槽函数
    void slot_clientDealData(char* buf,int nlen);               //客户端处理数据槽函数
    void slot_UserItemClicked();                                //单击好友头像处理槽函数
    void slot_SendChatContent(int id,QString content);          //发送信息处理槽函数
    void slot_delete();                                         //回收空间
    void slot_dealMenu(QAction*action);                         //处理菜单
    void slot_addFriendRq(QString name);                        //添加好友回复
    void slot_createRoom();                                     //创建房间
    void slot_joinRoom(quint32 id);                             //加入房间
    void slot_quitRoom(int id);                                 //退出房间
    void slot_openAudio();
    void slot_openVideo();
    void slot_closeAudio();
    void slot_closeVideo();
    void slot_videoItemClick();
    void slot_sendVideoFrame(QImage &);
    void slot_refreshImage(int id, QImage &image);
    void slot_sendAudioFrame(QByteArray);
    void slot_sendFileRq(int id,QString path);
    void slot_sendFileBlock(FileInfo fi);
    //void slot_addToFileList(int id, QString path);
    void slot_addToFileList(int id, QString path);
    void slot_updateUploadProcess(int pos, int max);
    void slot_downLoadFile();

public:
    void dealLoginRs(char* buf,int nlen);                       //处理登录回复
    void dealRegisterRs(char* buf,int nlen);                    //处理注册回复
    void dealForceOffline(char* buf,int nlen);                  //处理强制下线
    void dealAddFriendRq(char *buf,int nlen);                   //处理添加好友请求
    void dealAddFriendRs(char *buf,int nlen);                   //处理添加好友回复
    void dealFriendInfo(char *buf,int nlen);                    //处理好友信息
    void dealChatContentRq(char *buf,int nlen);                 //处理聊天内容请求
    void dealChatContentRs(char *buf,int nlen);                 //处理聊天内容回复
    void dealCreateRoomRs(char *buf,int nlen);                  //处理创建房间回复
    void dealJoinRoomRs(char *buf, int nlen);                   //处理加入房间回复
    void dealMemberRq(char *buf, int nlen);                     //处理成员信息请求
    void dealLeaveRs(char *buf, int nlen);                      //处理离开房间回复
    void dealVideoFrame(char *buf, int nlen);                   //处理视频帧
    void dealAudioFrame(char *buf, int nlen);                   //处理音频帧
    void dealUpLoadRs(char *buf, int nlen);                     //处理上传回复包
    void dealDownLoadFileInfo(char* buf,int nlen);              //处理下载文件信息包
    void dealDownloadFileRs(char* buf,int nlen);
    void dealDownloadFileBlockRq(char* buf,int nlen);
signals:
    void SIG_UpdateUploadProcess(int filePos,int fileSize);
private:
    Ui::WeTalk *ui;
public:
    int m_userID;                                               //用户id
    LoginDialog *m_login;                                       //登录窗口
    QMyTcpClient *m_client;                                     //客户端对象

    IMToolItem *m_userList;                                     //好友列表
    IMToolItem *m_videoList;                                    //播放列表


    ChatDialog* dialog;                                         //对话窗口
    char m_userName[MAX_SIZE];                                  //转存用户名
    QMenu* m_mainMenu;                                          //弹出菜单
    AddFriend* m_addFriend;                                     //添加好友窗口

    QMap<int,UserItem*> m_mapIDToUserItem;                      //id与用户列表的映射，子控件不需要回收
    QMap<int,ChatDialog*> m_mapIDToChatDialog;                  //id与对话窗口的映射，需要人为回收
    QMap<int,VideoItem*> m_mapIDToVideoItem;                    //用户id与房间的映射，不需要人为回收
    QMap<int,Audio_Write*> m_mapIDToAudioWrite;                 //用户id与音频输出的映射，回收写在退出房间
    QMap<int,IMToolItem*> m_mapIDToFileList;                    //用户id与文件列表的映射，子控件不需要回收
    QMap<int , FileInfo*> m_mapIDToFileInfo;                       //用户id与文件信息的映射，需要人为回收

    //QMap<int,VideoItem*> m_mapRoomIDToRoom;                   //一个用户只能加入一个房间，不需要在客户端存映射
    //QList<FileItem*> m_lstFileItem;                              //发送文件列表，需要人为回收
    quint32 m_roomID;                                           //房间id
    RoomDialog* m_roomDlg;                                      //房间对话框
    Video_Read* m_pVideoRead;                                   //读取视频对象
    Audio_Read* m_pAudioRead;                                   //读取音频对象

    QString savePath;


};

#endif // WETALK_H
