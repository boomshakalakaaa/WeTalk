#include "useritem.h"
#include "ui_useritem.h"
#include <QBitmap>

UserItem::UserItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserItem)
{
    ui->setupUi(this);
}

UserItem::~UserItem()
{
    delete ui;
}

void UserItem::setUserInfo(int userID,QString name,int nState,QString icon,QString feeling)
{
    m_userID = userID;
    m_username = name;
    m_icon = icon;
    m_feeling = feeling;
    m_state = nState;

    ui->lb_name->setText(m_username);
    ui->lb_feeling->setText(m_feeling);
    if(nState )
        ui->pb_icon->setIcon(QIcon(icon));
    else
    {
        QBitmap gray_picture;
        gray_picture.load(icon);
        ui->pb_icon->setIcon(QIcon(gray_picture));
    }
    this->repaint();    //立即更新视图    update是加入队列，等待合适时间更新
}
//点击头像
void UserItem::on_pb_icon_clicked()
{
    //emit SIG_UserItemClicked();
}
void UserItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit SIG_UserItemClicked();
}
