#include "wetalk.h"
#include "ui_wetalk.h"
#include <QMessageBox>
#include <QCryptographicHash>
#include "ui_chatdialog.h"
#include "ui_roomdialog.h"
#include "ui_fileitem.h"
#include <QTime>
#include <QInputDialog>
#include <QBuffer>
#include <QFileInfo>
#include <QFileDialog>

#define MD5_KEY (1234)

static QByteArray GetMD5(QString str)
{
    QString val = QString("%1 %2").arg(str).arg(MD5_KEY);
    //toLatin1 ASCII码 toLoacl8Bit Unicode码
    QByteArray buf = val.toLocal8Bit();
    QByteArray res = QCryptographicHash::hash(buf,QCryptographicHash::Md5);

    return res.toHex();
}

WeTalk::WeTalk(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WeTalk),m_userID(0),m_roomID(0),m_pAudioRead(NULL)
{
    ui->setupUi(this);

    m_login = new LoginDialog();
    m_login->show();
    connect(m_login,SIGNAL(SIG_loginCommit(QString,QString)),
            this,SLOT(slot_loginCommit(QString,QString)));        //登录处理
    connect(m_login,SIGNAL(SIG_registerCommit(QString,QString)),
            this,SLOT(slot_registerCommit(QString,QString)));     //注册处理

    //客户端连接写在显示的下面，否则在点击启动之后如果服务器半天连不上，就不会弹出显示登录界面
    m_client = new QMyTcpClient;
    if(m_client->InitNetWork(_DEF_SERVERIP,_DEF_PORT) == false)
    {
        QMessageBox::about(this,"提示","服务器连接失败");
    }

    connect(m_client,SIGNAL(SIG_ReadyData(char*,int)),            //接收数据处理
            this,SLOT(slot_clientDealData(char*,int)));

    //外部抽屉类 添加一个  “我的好友”列表
    m_userList = new IMToolItem("我的好友");
    ui->wdg_box->addItem(m_userList);

    //添加菜单
    m_mainMenu = new QMenu(this);
    m_mainMenu->addAction("添加好友");
    //添加分隔符
    m_mainMenu->addSeparator();
    m_mainMenu->addAction("创建房间");
    m_mainMenu->addAction("加入房间");

    //处理点击菜单选项后弹出对话框
    connect(m_mainMenu,SIGNAL(triggered(QAction*)),this,SLOT(slot_dealMenu(QAction*)));

    //初始化添加好友对话框，创建后先隐藏，等待点击后显示
    m_addFriend = new AddFriend;
    m_addFriend->hide();
    //处理点击添加按钮后 发送过来的添加好友信号
    connect(m_addFriend,SIGNAL(SIG_AddFriend(QString)),this,SLOT(slot_addFriendRq(QString)));

    //初始化房间
    m_roomDlg = new RoomDialog;
    //处理退出房间
    connect(m_roomDlg,SIGNAL(SIG_quitRoom(int)),this,SLOT(slot_quitRoom(int) ));
    //处理打开音频
    connect(m_roomDlg,SIGNAL(SIG_openAudio()),this,  SLOT(slot_openAudio()   ));
    //处理打开视频
    connect(m_roomDlg,SIGNAL(SIG_openVideo()),this,  SLOT(slot_openVideo()   ));
    //处理关闭音频
    connect(m_roomDlg,SIGNAL(SIG_closeAudio()),this, SLOT(slot_closeAudio()  ));
    //处理关闭视频
    connect(m_roomDlg,SIGNAL(SIG_closeVideo()),this,SLOT(slot_closeVideo()  ));

    //添加房间成员列表
    m_videoList = new IMToolItem("房间成员",this);
    m_roomDlg->getUi()->wdg_right->addItem(m_videoList);

    //处理读视频帧
    m_pVideoRead = new Video_Read;
    connect(m_pVideoRead,SIGNAL(SIG_sendVideoFrame(QImage&)),this,SLOT(slot_sendVideoFrame(QImage&)));


    connect(this ,SIGNAL( SIG_UpdateUploadProcess(int,int)), this, SLOT(slot_updateUploadProcess(int,int)));
}

WeTalk::~WeTalk()
{
//    slot_delete();
    delete ui;
}
//回收
void WeTalk::slot_delete()
{
    if(m_login)
        delete m_login;
    if(m_client)
        delete m_client;
    if(m_addFriend)
        delete m_addFriend;
    if(m_roomDlg)
        delete m_roomDlg;

    for(auto ite = m_mapIDToChatDialog.begin(); ite != m_mapIDToChatDialog.end(); ++ite)
    {
        delete *ite;
        *ite = 0;
    }
    m_mapIDToChatDialog.clear();
    if(m_pVideoRead)
    {
        m_pVideoRead->slot_closeVideo();
        delete m_pVideoRead;
    }

    for(auto ite = m_mapIDToFileInfo.begin(); ite!= m_mapIDToFileInfo.end(); ++ite)
    {
        delete (*ite);
        *ite = 0;
    }
    m_mapIDToFileInfo.clear();
}

//============================================客户端发送请求包============================================
//登录槽函数
void WeTalk::slot_loginCommit(QString name,QString password)
{
    STRU_LOGIN_RQ rq;
    //QString转stdstring，通过strcpy_s拷贝到rq中
    std::string strTmp = name.toStdString();
    char *buf = (char*)strTmp.c_str();
    strcpy_s(rq.m_szUser,buf);

    //    strTmp = password.toStdString();
    //    buf = (char*)strTmp.c_str();
    //    strcpy_s(rq.m_szPassword,buf);
    //转存用户名
    strcpy_s(m_userName,rq.m_szUser);
    QByteArray res = GetMD5(password);
    memcpy(rq.m_szPassword,res.data(),res.length());

    //客户端发送数据到服务器
    m_client->SendData((char*)&rq,sizeof(rq));
}
//注册槽函数
void WeTalk::slot_registerCommit(QString name,QString password)
{
    STRU_REGISTER_RQ rq;
    std::string strTmp = name.toStdString();
    char *buf = (char*)strTmp.c_str();
    strcpy_s(rq.m_szUser,buf);

    QByteArray res = GetMD5(password);
    memcpy(rq.m_szPassword,res.data(),res.length());
//    strTmp = password.toStdString();
//    buf = (char*)strTmp.c_str();
//    strcpy_s(rq.m_szPassword,buf);

    //客户端发送数据到服务器
    m_client->SendData((char*)&rq,sizeof(rq));
}
//点击头像处理槽函数
void WeTalk::slot_UserItemClicked()
{
    UserItem *item = (UserItem *)sender();
    if(m_mapIDToChatDialog.find(item->m_userID) != m_mapIDToChatDialog.end())
    {
        ChatDialog* dlg = m_mapIDToChatDialog[item->m_userID];
        dlg->show();
    }else
    {
        dialog = new ChatDialog;
        dialog->setInfo(item->m_userID,item->m_username,item->m_icon);
        dialog->hide();
        connect(dialog,SIGNAL(SIG_SendChatContent(int,QString)),
                this,SLOT(slot_SendChatContent(int,QString)));
//        connect(dialog,SIGNAL(SIG_SendFile(int)),
//                this,SLOT(slot_sendFileRq(int)));
        m_mapIDToChatDialog[dialog->m_id] = dialog;
    }

    //QMessageBox::about(this,"提示","当前用户"+item->m_username+"被点击");
}

//发送信息处理槽函数
void WeTalk::slot_SendChatContent(int id,QString content)
{
//    //聊天请求结果
//            #define normal_msg      0
//            #define user_offline    1
//            #define offline_msg     2
    //封装 聊天内容包
    STRU_CHAT_RQ rq;
    rq.m_userID = m_userID;
    rq.m_friendID = id;
    rq.m_result = normal_msg;
    std::string tmp = content.toStdString();
    const char* buf = tmp.c_str();
    strcpy_s(rq.m_ChatContent,buf);

    m_client->SendData((char*)&rq,sizeof(rq));
}
//客户端接收服务器返回包处理数据
void WeTalk::slot_clientDealData(char* buf,int nlen)
{
    int nType = *(int*)buf; //按4字节取

    switch(nType)
    {
    case DEF_PACK_REGISTER_RS:
        dealRegisterRs(buf,nlen);
        break;
    case DEF_PACK_LOGIN_RS:
        dealLoginRs(buf,nlen);
        break;
    case DEF_PACK_FORCE_OFFLINE:
        dealForceOffline(buf,nlen);
        break;
    case DEF_PACK_ADD_FRIEND_RQ:
        dealAddFriendRq(buf,nlen);
        break;
    case DEF_PACK_ADD_FRIEND_RS:
        dealAddFriendRs(buf,nlen);
        break;
    case DEF_PACK_FRIEND_INFO:
        dealFriendInfo(buf,nlen);
        break;
    case DEF_PACK_CHAT_RQ:
        dealChatContentRq(buf,nlen);
        break;
    case DEF_PACK_CHAT_RS:
        dealChatContentRs(buf,nlen);
        break;
    case DEF_PACK_CREATEROOM_RS:
        dealCreateRoomRs(buf,nlen);
        break;
    case DEF_PACK_JOINROOM_RS:
        dealJoinRoomRs(buf,nlen);
        break;
    case DEF_PACK_ROOM_MEMBER:
        dealMemberRq(buf,nlen);
        break;
    case DEF_PACK_LEAVEROOM_RS:
        dealLeaveRs(buf,nlen);
        break;
    case DEF_PACK_VIDEO_FRAME:
        dealVideoFrame(buf,nlen);
        break;
    case DEF_PACK_AUDIO_FRAME:
        dealAudioFrame(buf,nlen);
        break;
    case DEF_PACK_FILE_UPLOAD_RS:
        dealUpLoadRs(buf,nlen);
        break;
    case DEF_PACK_DOWNLOAD_FILEINFO:
        dealDownLoadFileInfo(buf,nlen);
        break;
    case DEF_PACK_DOWNLOAD_RS:
        dealDownloadFileRs(buf,nlen);
        break;
    case DEF_PACK_FILEBLOCK_DOWNLOAD_RQ:
        dealDownloadFileBlockRq(buf,nlen);
        break;
    }
    delete []buf;
}
//菜单响应处理
void WeTalk::slot_dealMenu(QAction* action)
{
    if(action->text() == "添加好友")
    {
        m_addFriend->clear();
        m_addFriend->show();

    }else if(action->text() == "创建房间")
    {
        if(m_roomID == 0)
        {
            slot_createRoom();
        }else
        {
            QMessageBox::about(this,"提示","已加入房间！");

            return;
        }

    }else if(action->text() == "加入房间")
    {
        if(m_roomID != 0)
        {
            QMessageBox::about(this,"提示","已加入房间！");
            return;
        }
        //创建对话框 输入正则匹配 0-9匹配1-5次
        QString id = QInputDialog::getText(this,"加入房间","请输入房间号");
        QRegExp reg("[0-9]{1,6}");
        if(reg.exactMatch(id) )
             slot_joinRoom( id.toInt());
        else
            QMessageBox::about(this, "提示","房间号非法");

    }

}
//发送创建房间请求
void WeTalk::slot_createRoom()
{
    STRU_CREATEROOM_RQ rq;
    rq.m_UserID = m_userID;
    m_client->SendData((char*)&rq,sizeof(rq));
}
//发送加入房间请求
void WeTalk::slot_joinRoom(quint32 id)
{
    STRU_JOINROOM_RQ rq;
    rq.m_UserID = m_userID;
    rq.m_RoomID = id;
    m_client->SendData((char*)&rq,sizeof(rq));
}
//退出房间请求
void WeTalk::slot_quitRoom(int id)
{
    //封包
    STRU_LEAVEROOM_RQ rq;
    rq.m_nUserId = m_userID;
    rq.m_RoomId = m_roomID;
    m_client->SendData((char*)&rq,sizeof(rq));

    //停止视频采集
    this->slot_closeVideo();
    //停止音频采集
    this->slot_closeAudio();
    //销毁显示模块
    for(auto ite = m_mapIDToVideoItem.begin();ite != m_mapIDToVideoItem.end();)
    {
        VideoItem* item = *ite;
        //从显示界面上清除
        m_videoList->removeItem(item);
        delete item;
        //从map中删除
        ite = m_mapIDToVideoItem.erase(ite);
    }
    //销毁声音播放
    for(auto ite = m_mapIDToAudioWrite.begin();ite != m_mapIDToAudioWrite.end();)
    {
        delete *ite;
        ite = m_mapIDToAudioWrite.erase(ite);
    }
    //房间隐藏
    m_roomDlg->hide();
    //房间标识清空
    m_roomID = 0;
}

//打开音频
void WeTalk::slot_openAudio()
{
    if(m_pAudioRead)
    {
        m_pAudioRead->ResumeAudio();
    }
}
//打开视频
void WeTalk::slot_openVideo()
{
    if(m_pVideoRead)
    {
        m_pVideoRead->slot_openVideo();
    }
}
//关闭音频
void WeTalk::slot_closeAudio()
{
    if(m_pAudioRead)
    {
        m_pAudioRead->PauseAudio();
    }
}
//关闭视频
void WeTalk::slot_closeVideo()
{
    if(m_pVideoRead)
    {
        m_pVideoRead->slot_closeVideo();
    }
}

//视频采集的图片发送出去
void WeTalk::slot_sendVideoFrame(QImage& image)
{
    //先显示到自己的控件上
    this->slot_refreshImage(this->m_userID,image);
    //将图片编码为jpeg格式，QBuffer与QByteArray关联
    QByteArray bt;
    QBuffer buf(&bt);
    //将图片转成为jpeg格式
    image.save(&buf,"JPEG");
    //通过指针偏移，每4个字节写一次
    int nlen = 12 + bt.size();
    char* szbuf = new char[nlen];
    char* tmp = szbuf;
    *(int*)tmp = DEF_PACK_VIDEO_FRAME;
    tmp += sizeof(int);
    *(int*)tmp = m_userID;
    tmp += sizeof(int);
    *(int*)tmp = m_roomID;
    tmp += sizeof(int);
    //将存储到QByteArray中的数据，拷贝到tmp中
    memcpy(tmp,bt.data(),bt.size());
    //发送视频帧到服务器
    m_client->SendData(szbuf,nlen);
    delete []szbuf;
}

//音频采集的帧发送出去
void WeTalk::slot_sendAudioFrame(QByteArray ba)
{
    //通过指针偏移，每4个字节写一次
    int nlen = 12 + ba.size();
    char* szbuf = new char[nlen];
    char* tmp = szbuf;
    *(int*)tmp = DEF_PACK_AUDIO_FRAME;
    tmp += sizeof(int);
    *(int*)tmp = m_userID;
    tmp += sizeof(int);
    *(int*)tmp = m_roomID;
    tmp += sizeof(int);
    //将存储到QByteArray中的数据，拷贝到tmp中
    memcpy(tmp,ba.data(),ba.size());
    //发送视频帧到服务器
    m_client->SendData(szbuf,nlen);
    delete []szbuf;
}

//向服务器发送添加好友请求
void WeTalk::slot_addFriendRq(QString name)
{
    //错误处理
    //1.自己不可添加自己
    QString Username = m_userName;
    if(name == Username)
    {
        QMessageBox::about(this,"提示","不可以添加自己为好友！");
        return ;
    }
    //2.已经是好友不能重复添加
    for(auto ite = m_mapIDToUserItem.begin();ite != m_mapIDToUserItem.end();++ite)
    {
        UserItem* item = *ite;
        if(name == item->m_username)
        {
            QMessageBox::about(this,"提示","已经存在此好友，不可重复添加！");
            return ;
        }
    }
    //发送添加请求 请求内容包括 用户id，用户名，请求添加的用户名
    STRU_ADD_FRIEND_RQ rq;
    rq.m_userID = m_userID;
    strcpy_s(rq.m_szUserName,m_userName);
    std::string tmp = name.toStdString();
    const char* buf = tmp.c_str();
    strcpy_s(rq.m_szAddFriendName,buf);

    m_client->SendData((char*)&rq,sizeof(rq));
}

//发文件请求
void WeTalk::slot_sendFileRq(int id,QString path)
{
    //封包，先发文件信息
    STRU_UPLOAD_RQ rq;
    //发送者id
    rq.m_senderId = m_userID;
    rq.m_recverId = id;
    //文件id
    rq.m_nFileId = qrand() % 1000;
    //文件大小
    QFileInfo imgInfo(path);
    rq.m_nFileSize = imgInfo.size();
    //文件名
    QString name = imgInfo.baseName();
    std::string tmp = name.toStdString();
    const char* buf = tmp.c_str();
    strcpy_s(rq.m_szFileName,buf);
    //文件类型
    QString suffix = imgInfo.suffix();
    tmp = suffix.toStdString();
    buf = tmp.c_str();
    strcpy_s(rq.m_szFileType,buf);

    //发文件信息包
    m_client->SendData((char*)&rq,sizeof(rq));

    //发送文件
    FileInfo fi;
    fi.m_nFileId = rq.m_nFileId;
    fi.m_nUserId = rq.m_senderId;
    fi.m_nRecverId = rq.m_recverId;
    fi.m_nPos = 0;
    fi.m_nFilesize = rq.m_nFileSize;
    strcpy_s(fi.m_szFileName,rq.m_szFileName);
    std::string str = path.toStdString();
    const char* ch = str.c_str();
    strcpy_s(fi.m_szFilePath,ch);
    strcpy_s(fi.m_szFileType,rq.m_szFileType);
    fi.pFile = new QFile(path);
    this->slot_sendFileBlock(fi);
}

//发送文件
void WeTalk::slot_sendFileBlock(FileInfo fi)
{
    FileInfo * pInfo = &fi;
    if(fi.pFile->open(QIODevice::ReadOnly))
    {
        while(1)
        {
            //文件内容 内容长度 文件Id,文件位置--传输请求
            STRU_UPLOAD_FILEBLOCK_RQ blockrq;
            //按MAX_CONTENT_LEN长度，读文件到m_szFileContent中，返回实际读取长度
            int64_t nRelReadNum = fi.pFile->read(blockrq.m_szFileContent ,MAX_CONTENT_LEN );
            //发送包大小 = 实际读取文件长度
            blockrq.m_nBlockLen = nRelReadNum;
            //发送包文件id
            blockrq.m_nFileId = fi.m_nFileId;
            //发送包发送者id
            blockrq.m_nUserId = fi.m_nUserId;
            printf("Current nRelReadNum:%d\n",blockrq.m_nBlockLen);
            //发文件内容包
            m_client->SendData((char*)&blockrq,sizeof(blockrq));
            //文件位置偏移，下一次从此位置继续读
            pInfo->m_nPos += nRelReadNum;
            //设置进度
            emit SIG_UpdateUploadProcess(pInfo->m_nPos,pInfo->m_nFilesize);
            //判断终止条件
            if(pInfo->m_nPos >= pInfo->m_nFilesize)
            {
                pInfo->pFile->close();
                delete  pInfo->pFile ;
                break;
            }
        }
    }
}

//============================================处理服务器回复包============================================
//处理登录回复
void WeTalk::dealLoginRs(char* buf,int nlen)
{
//    #define userid_no_exist      0
//    #define password_error       1
//    #define login_sucess         2
//    #define user_online          3
    STRU_LOGIN_RS* rs = (STRU_LOGIN_RS*)buf;
    switch(rs->m_lResult)
    {
    case userid_no_exist:
        QMessageBox::information(this->m_login,"提示","该用户不存在，请注册！");
        break;
    case password_error:
        QMessageBox::information(this->m_login,"提示","密码错误，请重新输入！");
        break;
    case login_sucess:
        this->m_login->hide();
        this->show();
        this->ui->lb_name->setText(m_userName);
        //this->ui->pb_icon->setIcon(rs->);
        m_userID = rs->m_UserID;
        break;
    }
}
//处理注册回复
void WeTalk::dealRegisterRs(char* buf,int nlen)
{
//      #define userid_is_exist      0
//      #define register_sucess      1
    STRU_REGISTER_RS* rs = (STRU_REGISTER_RS*)buf;
    switch(rs->m_lResult)
    {
    case userid_is_exist:
        QMessageBox::information(this->m_login,"提示","该用户名已存在，请重新输入！");
        break;
    case register_sucess:
        QMessageBox::information(this->m_login,"提示","注册成功！");
        break;
    }
}
//处理强制下线
void WeTalk::dealForceOffline(char* buf,int nlen)
{
    STRU_FORCE_OFFLINE* off = (STRU_FORCE_OFFLINE*)buf;
    if(off->m_UserID == m_userID)
    {
        QMessageBox::information(this,"提示","异地登录，强制下线!");
        this->close();
    }
}
//处理服务器发来的添加好友请求
void WeTalk::dealAddFriendRq(char *buf, int nlen)
{
    STRU_ADD_FRIEND_RQ* rq = (STRU_ADD_FRIEND_RQ*)buf;
    QString name = rq->m_szUserName;
    STRU_ADD_FRIEND_RS rs;
    rs.m_friendID = m_userID;          //被动方，添加的朋友的id，是当前用户的id
    rs.m_userID = rq->m_userID;        //主动方，发送添加请求的人的id
    strcpy_s(rs.szAddFriendName,rq->m_szAddFriendName);

    if(QMessageBox::question(this,"添加好友",QString("是否添加【%1】为好友？").arg(name)) == QMessageBox::Yes)
    {
        rs.m_result = add_success;

    }else
    {
        rs.m_result = user_refused;
    }
    m_client->SendData((char*)&rs,sizeof(rs));
}

//处理服务器发回的添加好友回复
void WeTalk::dealAddFriendRs(char *buf, int nlen)
{
//    //添加好友结果
//    #define no_this_user         0
//    #define user_refused         1
//    #define add_success          2
//    #define user_is_offline      3
    STRU_ADD_FRIEND_RS* rs = (STRU_ADD_FRIEND_RS*)buf;

    switch(rs->m_result)
    {
    case no_this_user:
        QMessageBox::about(this,"提示","不存在该用户！");
        break;
    case user_refused:
        QMessageBox::about(this,"提示",QString("用户【%1】已拒绝！").arg(rs->szAddFriendName));
        break;
    case add_success:
        QMessageBox::about(this,"提示",QString("添加【%1】成功！").arg(rs->szAddFriendName));
        break;
    case user_is_offline:
        QMessageBox::about(this,"提示",QString("用户【%1】不在线！").arg(rs->szAddFriendName));
        break;
    }
}

//好友信息更新
void WeTalk::dealFriendInfo(char* buf,int nlen)
{
    STRU_FRIEND_INFO* info = (STRU_FRIEND_INFO*)buf;

    //查找id对应的用户信息
    if(m_mapIDToUserItem.find(info->m_userID) != m_mapIDToUserItem.end())
    {
        //如果有，更新
        UserItem* item = m_mapIDToUserItem[info->m_userID];
        item->setUserInfo(info->m_userID,info->m_szName,info->m_state/*,
                          QString(":/tx/%1.png").arg(info->m_iconID),info->m_feeling*/);

        //查找id对应的对话框，如果找到，直接赋值
        if( m_mapIDToChatDialog.find( info->m_userID ) != m_mapIDToChatDialog.end() )
        {
            ChatDialog* chatdlg = m_mapIDToChatDialog[info->m_userID];
            chatdlg->setInfo(info->m_userID,info->m_szName,QString(":/tx/%1.png").arg(info->m_iconID));
//            //查找id对应的文件列表
//            if(m_mapIDToFileList.find(info->m_userID) != m_mapIDToFileList.end())
//            {
//                //找到了
//                IMToolItem* m_fileList = m_mapIDToFileList[info->m_userID];
//                chatdlg->getUi()->wdg_sendfile->addItem(m_fileList);

//            }else
//            {
//                //没找到
//                //创建文件列表，把文件列表显示到控件中，将文件列表通过id映射添加到map中
//                IMToolItem *m_fileList = new IMToolItem("文件列表");
//                chatdlg->getUi()->wdg_sendfile->addItem(m_fileList);
//                m_mapIDToFileList[info->m_userID] = m_fileList;
//            }
            connect(chatdlg,SIGNAL(SIG_addToFileList(int,QString)),
                    this,SLOT(slot_addToFileList(int,QString)));

        }

    }else
    {
        //没有，添加并创建聊天窗口
        //设置用户信息
        UserItem* item = new UserItem;
        item->setUserInfo(info->m_userID,info->m_szName,info->m_state/*,
                          QString(":/tx/%1.png").arg(info->m_iconID),info->m_feeling*/);

        connect(item,SIGNAL(SIG_UserItemClicked()),this,SLOT(slot_UserItemClicked()));
        //通过id与信息映射 添加用户信息
        m_mapIDToUserItem[info->m_userID] = item;

        //创建、设置、添加聊天窗口
        ChatDialog* chatdlg = new ChatDialog;
        chatdlg->setInfo(info->m_userID,info->m_szName,QString(":/tx/%1.png").arg(info->m_iconID));

        //处理发送消息内容信号
        connect(chatdlg,SIGNAL(SIG_SendChatContent(int,QString)),this,SLOT(slot_SendChatContent(int,QString)));

        //创建文件列表，把文件列表显示到控件中，将文件列表通过id映射添加到map中
        IMToolItem *m_fileList = new IMToolItem("文件列表");
        chatdlg->getUi()->wdg_sendfile->addItem(m_fileList);
        m_mapIDToFileList[info->m_userID] = m_fileList;

        //处理发送文件信号
        connect(chatdlg,SIGNAL(SIG_addToFileList(int,QString)),
                this,SLOT(slot_addToFileList(int,QString)));

        m_mapIDToChatDialog[info->m_userID] = chatdlg;
        //放到抽屉类的控件中
        m_userList->addItem(item);
    }

}

//处理聊天内容请求包
void WeTalk::dealChatContentRq(char *buf, int nlen)
{
    //解包
    STRU_CHAT_RQ* rq = (STRU_CHAT_RQ*)buf;

    switch(rq->m_result)
    {
    case normal_msg:
    {
        //通过id查对话框
        if(m_mapIDToChatDialog.find(rq->m_userID) != m_mapIDToChatDialog.end())
        {
            //找到对应的对话框，把聊天信息粘贴到窗口
            ChatDialog* chatdlg = m_mapIDToChatDialog[rq->m_userID];
            chatdlg->getUi()->tb_chat->append(QString("【%1】%2").arg(chatdlg->m_name)
                                              .arg((QTime::currentTime().toString("hh:mm:ss"))));
            QString content = rq->m_ChatContent;
            chatdlg->getUi()->tb_chat->append(content);
            chatdlg->getUi()->tb_chat->append("");
        }
        break;
    }
    case offline_msg:
    {
        //通过id查对话框
        if(m_mapIDToChatDialog.find(rq->m_friendID) != m_mapIDToChatDialog.end())
        {
            //找到对应的对话框，把聊天信息粘贴到窗口
            ChatDialog* chatdlg = m_mapIDToChatDialog[rq->m_friendID];
            chatdlg->getUi()->tb_chat->append(QString("【%1】%2").arg(chatdlg->m_name)
                                              .arg((QTime::currentTime().toString("hh:mm:ss"))));
            QString content = rq->m_ChatContent;
            chatdlg->getUi()->tb_chat->append(content);
            chatdlg->getUi()->tb_chat->append("");
        }
        break;
    }
    }


}

//处理聊天内容回复包
void WeTalk::dealChatContentRs(char *buf,int nlen)
{
    //解包
    STRU_CHAT_RS* rs = (STRU_CHAT_RS*)buf;
    if(rs->m_result == user_is_offline)
    {
        //如果对方不在线,通过id查对话框
        if(m_mapIDToChatDialog.find(rs->m_friendID) != m_mapIDToChatDialog.end())
        {
            ChatDialog* chatdlg = m_mapIDToChatDialog[rs->m_friendID];
            chatdlg->getUi()->tb_chat->append(QString("【系统】%1").arg(QTime::currentTime().toString("hh:mm:ss")));
            chatdlg->getUi()->tb_chat->append("用户未在线，消息已转存离线数据库");
            chatdlg->getUi()->tb_chat->append("");
        }
    }
}
//处理创建房间回复
void WeTalk::dealCreateRoomRs(char *buf,int nlen)
{
    //拆包
    STRU_CREATEROOM_RS* rs = (STRU_CREATEROOM_RS*)buf;
    //成功
    if(rs->m_lResult)
    {
        //设置房间标志，显示房间
        m_roomID = rs->m_RoomId;
        m_roomDlg->setRoomid(rs->m_RoomId);
        //显示视频创建
        if(m_mapIDToVideoItem.find(this->m_userID) == m_mapIDToVideoItem.end())
        {
            //显示自己
            VideoItem* item = new VideoItem;
            item->setInfo(this->m_userName,this->m_userID);
            connect(item,SIGNAL(SIG_itemClicked()),this,SLOT(slot_videoItemClick()));
            //显示到box
            m_videoList->addItem(item);
            m_mapIDToVideoItem[m_userID] = item;
        }

        m_roomDlg->show();
        //初始化，链接
        m_pAudioRead = new Audio_Read;
        connect(m_pAudioRead,SIGNAL(sig_net_tx_frame(QByteArray)),this,SLOT(slot_sendAudioFrame(QByteArray)));
    }else//失败
    {
        QMessageBox::about(this,"提示","创建房间失败");
        return;
    }
}

//处理加入房间回复
void WeTalk::dealJoinRoomRs(char *buf,int nlen)
{
    //拆包
    STRU_JOINROOM_RS* rs = (STRU_JOINROOM_RS*)buf;
    //成功
    if(rs->m_lResult)
    {
        //设置房间标志，显示房间
        m_roomID = rs->m_roomID;
        m_roomDlg->setRoomid(rs->m_roomID);
        //显示视频创建
        if(m_mapIDToVideoItem.find(this->m_userID) == m_mapIDToVideoItem.end())
        {
            //显示自己
            VideoItem* item = new VideoItem;
            item->setInfo(this->m_userName,this->m_userID);
            connect(item,SIGNAL(SIG_itemClicked()),this,SLOT(slot_videoItemClick()));
            //显示到box
            m_videoList->addItem(item);
            m_mapIDToVideoItem[m_userID] = item;

        }
        m_roomDlg->show();
        //创建音频采集
        m_pAudioRead = new Audio_Read;
        connect(m_pAudioRead,SIGNAL(sig_net_tx_frame(QByteArray)),this,SLOT(slot_sendAudioFrame(QByteArray)));
    }else//失败
    {
        QMessageBox::about(this,"提示","创建房间失败");
        return;
    }
}

//处理离开房间回复
void WeTalk:: dealLeaveRs(char *buf,int nlen)
{
    //解包
    STRU_LEAVEROOM_RS* rs = (STRU_LEAVEROOM_RS*)buf;
    auto ite = m_mapIDToVideoItem.find(rs->m_UserID);
    //清除视频模块
    if(ite != m_mapIDToVideoItem.end())
    {
        //从界面上移除
        VideoItem* item = m_mapIDToVideoItem[rs->m_UserID];
        m_videoList->removeItem(item);
        delete item;
        m_mapIDToVideoItem.erase(ite);
    }
    //清除音频模块

}


//处理成员信息请求
void WeTalk::dealMemberRq(char *buf,int nlen)
{
    //解包
    STRU_ROOM_MEMBER_RQ* rq = (STRU_ROOM_MEMBER_RQ*)buf;
    //创建对应的显示模块
    if(m_mapIDToVideoItem.find(rq->m_UserID) == m_mapIDToVideoItem.end())
    {
        VideoItem* item = new VideoItem;
        item->setInfo(rq->m_szUser,rq->m_UserID);
        connect(item,SIGNAL(SIG_itemClicked()),this,SLOT(slot_videoItemClick()));
        m_videoList->addItem(item);
        m_mapIDToVideoItem[rq->m_UserID] = item;
    }

    //创建对应的声音模块
    if(m_mapIDToAudioWrite.find(rq->m_UserID) == m_mapIDToAudioWrite.end())
    {
        //在map中没有找到对应的映射
        Audio_Write* audioWrite = new Audio_Write;
        m_mapIDToAudioWrite[rq->m_UserID] = audioWrite;

    }

}

//处理服务器发来的视频帧
void WeTalk::dealVideoFrame(char* buf,int nlen)
{
    //拆包
    char* tmp = buf;
    //跳过前四个字节的协议类型
    tmp += sizeof(int);
    //按4个字节取 id，房间号，视频帧
    int userId = *(int*)tmp;
    tmp += sizeof(int);
    int roomId = *(int*)tmp;
    tmp += sizeof(int);
    //此时tmp指向图片的缓冲区，将它变为图片image
    QByteArray bt(tmp,nlen-12);  //按长度拷贝到QByteArray中，前12字节为其它参数
    QImage img;
    img.loadFromData(bt);

    //显示到控件上
    this->slot_refreshImage(userId,img);
}
//处理服务器发来的音频帧
void WeTalk::dealAudioFrame(char* buf,int nlen)
{
    //拆包
    char* tmp = buf;
    //跳过前四个字节的协议类型
    tmp += sizeof(int);
    //按4个字节取 id，房间号，视频帧
    int userId = *(int*)tmp;
    tmp += sizeof(int);
    int roomId = *(int*)tmp;
    tmp += sizeof(int);

    //找map，对应的音频输出
    if(m_mapIDToAudioWrite.find(userId) != m_mapIDToAudioWrite.end())
    {
        QByteArray bt(tmp,nlen-12);
        Audio_Write* pAudioWrite = m_mapIDToAudioWrite[userId];
        //播放音频帧
        pAudioWrite->slot_net_rx(bt);
    }

}

//处理上传回复包，发送结果
void WeTalk::dealUpLoadRs(char* buf,int nlen)
{
    //拆包
    STRU_UPLOAD_RS* rs = (STRU_UPLOAD_RS*)buf;
    if(rs->m_nResult)
    {
        QMessageBox::about(this,"提示","已上传至离线文件服务器");
    }else
    {
        QMessageBox::about(this,"提示","发送失败");
    }
    delete []buf;
}
void WeTalk::dealDownLoadFileInfo(char* buf,int nlen)
{
    //拆包
    STRU_DOWNLOAD_FILEINFO* rs = (STRU_DOWNLOAD_FILEINFO*)buf;
    //根据发送者id找到对应的对话框显示
    if(m_mapIDToChatDialog.find(rs->m_nSenderId) != m_mapIDToChatDialog.end())
    {
        if(m_mapIDToFileList.find(rs->m_nSenderId) != m_mapIDToFileList.end())
        {
            FileItem* item = new FileItem;
            //文件大小
            int size = rs->m_nFileSize;
            //文件名
            strcat(rs->m_szFileName,".");
            strcat(rs->m_szFileName, rs->m_szFileType);
            QString name = rs->m_szFileName;
            //item->setinfo(name,size);
            item->setInfo(name,size,false);
            //文件项填入到对应id的文件列表中
            m_mapIDToFileList[rs->m_nSenderId]->addItem(item);
            connect(item->getUi()->pb_recv,SIGNAL(clicked()),this,SLOT(slot_downLoadFile()));
        }
    }
    delete []buf;
}
void WeTalk::dealDownloadFileRs(char* buf,int nlen)
{
    //拆包
    STRU_DOWNLOAD_RS* rs = (STRU_DOWNLOAD_RS*)buf;
    FileInfo *info = new FileInfo;
    //存储文件信息
    info->m_nFileId = rs->m_nFileId;
    info->m_nFilesize = rs->m_nFileSize;
    info->m_nPos = 0;
    strcpy_s(info->m_szFileName,rs->m_szFileName);
    strcpy_s(info->m_szFileType,rs->m_szFileType);
    std::string str = savePath.toStdString();
    const char* ch = str.c_str();
    strcpy_s(info->m_szFilePath,ch);

    //处理文件的存储路径
    QDir dir;
    dir.setPath(info->m_szFilePath);
    //看路径是否存在，如果不存在，创建
    if( !dir.exists( info->m_szFilePath ) )
        dir.mkdir(info->m_szFilePath);

    //存在，通过路径+文件名 新建一个文件 返回文件指针
    strcat(info->m_szFilePath,"/");
    strcat(info->m_szFilePath,info->m_szFileName);
    strcat(info->m_szFilePath,".");
    strcat(info->m_szFilePath,info->m_szFileType);
    QString file_name(info->m_szFilePath );
    info->pFile = new QFile(file_name);
    //通过文件指针 打开文件
    if( info->pFile->open( QIODevice::WriteOnly) )
    {
         m_mapIDToFileInfo[ rs->m_nFileId ] = info;
    }
    delete [] buf;
}

void WeTalk::dealDownloadFileBlockRq(char* buf,int nlen)
{
    //1. 拆包 , 查找map 找对应info
    STRU_DOWNLOAD_FILEBLOCK_RQ *rq = (STRU_DOWNLOAD_FILEBLOCK_RQ *)buf;

    STRU_DOWNLOAD_FILEBLOCK_RS rs;
    rs.m_nFileId = rq->m_nFileId;
    rs.m_nUserId = rq->m_nUserId;
    rs.m_nResult = _downloadfileblock_fail ;

    FileInfo * info = 0;
    INT64 nLen = 0 ;

    auto ite = this->m_mapIDToFileInfo.find(rq->m_nFileId);
    if( ite != this->m_mapIDToFileInfo.end() )
    {
        info = this->m_mapIDToFileInfo[rq->m_nFileId];

        //写入
        nLen = info->pFile->write(rq->m_szFileContent,rq->m_nBlockLen  );

        rs.m_nBlockLen = nLen;
        qDebug()<< "id："<<rq->m_nFileId<<"   "<< "BlockLen："<<rq->m_nBlockLen;
        info->m_nPos += nLen;
        //文件结束关闭
        if(  rq->m_nBlockLen < MAX_CONTENT_LEN || info->m_nPos >=  info->m_nFilesize )
        {
            rs.m_nResult = _downloadfileblock_success;
            QMessageBox::about(this,"提示","接收完毕");
            //关闭文件 , 删除节点
            info->pFile->close();
            delete info->pFile;
            info->pFile = 0;

        }
    }
    //回复
    m_client->SendData((char*)&rs , sizeof(rs));
}

//============================================ui界面============================================

//点击好友页
void WeTalk::on_tb_friend_clicked()
{
    ui->tb_friend->setChecked(true);
    ui->tb_group->setChecked(false);
    ui->tb_zone->setChecked(false);

    ui->stk_page->setCurrentIndex(0);
}
//点击群组
void WeTalk::on_tb_group_clicked()
{
    ui->tb_friend->setChecked(false);
    ui->tb_group->setChecked(true);
    ui->tb_zone->setChecked(false);

    ui->stk_page->setCurrentIndex(1);
}
//点击空间
void WeTalk::on_tb_zone_clicked()
{
    ui->tb_friend->setChecked(false);
    ui->tb_group->setChecked(false);
    ui->tb_zone->setChecked(true);

    ui->stk_page->setCurrentIndex(2);
}


//点击按钮退出
void WeTalk::on_pb_close_clicked()
{
    if(QMessageBox::question(this,"提示","是否退出应用？") == QMessageBox::Yes)
    {
        //调用close会跳转到close事件
        //强制退出时调用this->close直接跳转到close事件
        this->close();
    }
}
//关闭事件回收
void WeTalk::closeEvent(QCloseEvent *event)
{
    //发送离线消息
    this->slot_delete();
    //等待消息缓冲区的内容发送完再关闭
    Sleep(500);
    event->accept();
}

//点击弹出菜单
void WeTalk::on_pb_menu_clicked()
{
    m_mainMenu->exec(QCursor::pos());
}


//点击wdg_right显示控件    -->   设置wdg_left预览的信息（用户名、id）
void WeTalk::slot_videoItemClick()
{
    VideoItem *item = (VideoItem *)sender();
    if(m_mapIDToVideoItem.find(this->m_userID) != m_mapIDToVideoItem.end())
    {
        m_roomDlg->getUi()->wdg_left->setInfo(m_mapIDToVideoItem[item->m_id]->m_name,item->m_id);
    }else
    {
        m_roomDlg->getUi()->wdg_left->setInfo("---",item->m_id);
    }
}

//刷新左侧控件的预览图
void WeTalk::slot_refreshImage(int id,QImage& image)
{
    //在map中查找id对应的控件
    if(m_mapIDToVideoItem.find(id) != m_mapIDToVideoItem.end())
    {
        //设置控件中的图片
        VideoItem* item = m_mapIDToVideoItem[id];
        item->setImage(image);

        //判断当前预览的是否是当前id，如果是，也设置图片
        if(m_roomDlg->getUi()->wdg_left->m_id == id)
        {
            m_roomDlg->getUi()->wdg_left->setImage(image);
        }
    }

}

//显示发送文件进度条
void WeTalk::slot_addToFileList(int id,QString path)
{
    FileItem* item = new FileItem;
    //文件大小
    QFileInfo imgInfo(path);
    int size = imgInfo.size();
    //文件名
    QString name = imgInfo.fileName();
    item->setInfo(name,size,true);
    //文件项填入到对应id的文件列表中
    m_mapIDToFileList[id]->addItem(item);
    //点击按键，跳转到槽函数
    connect(item->getUi()->pb_send,&QPushButton::clicked,[this,id,path](){slot_sendFileRq(id,path);});
}

//处理下载文件，发下载请求包
void WeTalk::slot_downLoadFile()
{
    savePath = QFileDialog::getExistingDirectory(this, "请选择存储路径","/");
    if (savePath.isEmpty())
    {
        return;
    }
    else
    {
        STRU_DOWNLOAD_RQ rq;
        rq.m_nUserId = m_userID;
        m_client->SendData((char*)&rq,sizeof(rq));
    }
}

void WeTalk::slot_updateUploadProcess(int pos,int max)
{
    //updateUploadProcess
}
