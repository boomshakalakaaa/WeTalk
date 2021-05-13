#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>

namespace Ui {
class UserItem;
}

class UserItem : public QWidget
{
    Q_OBJECT

public:
    explicit UserItem(QWidget *parent = 0);
    ~UserItem();

private:
    Ui::UserItem *ui;

signals:
    void SIG_UserItemClicked();
public slots:
    void mouseDoubleClickEvent(QMouseEvent *event);     //双击事件
    void on_pb_icon_clicked();                          //单击头像
public:
    //用户资料
    int m_userID;           //用户id
    QString m_username;     //用户名
    QString m_icon;         //用户图像
    QString m_feeling;      //用户心情
    int m_state;            //用户状态
    void setUserInfo(int userID,QString name,int nState,
                     QString icon = QString(":/tx/8.png"),QString feeling = QString("比较懒，什么也没写"));
};

#endif // USERITEM_H
