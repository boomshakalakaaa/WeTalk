#include "roomdialog.h"
#include "ui_roomdialog.h"
#include <QMessageBox>

RoomDialog::RoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomDialog),m_roomid(0)
{
    ui->setupUi(this);
}

RoomDialog::~RoomDialog()
{
    delete ui;
}
//最小化
void RoomDialog::on_pb_min_clicked()
{
    this->showMinimized();
}
//关闭
void RoomDialog::on_pb_close_clicked()
{
    this->on_pb_quitRoom_clicked();
}
//退出房间
void RoomDialog::on_pb_quitRoom_clicked()
{
    if(0 == m_roomid)
    {
        QMessageBox::about(this,"提示","请先加入房间");
        return ;
    }
    if(QMessageBox::question(this,"退出房间","是否确认退出房间") == QMessageBox::No) return;
    //清空控件
    this->clear();
    //发信号
    emit SIG_quitRoom(m_roomid);
}
//清除
void RoomDialog::clear()
{
    ui->lb_title->setText("房间号:");
    ui->pb_openAudio->setText("打开音频");
    ui->pb_openVideo->setText("打开视频");
    m_roomid = 0;
}
//打开视频
void RoomDialog::on_pb_openVideo_clicked()
{
    if(m_roomid == 0)
    {
        QMessageBox::about(this,"提示","请加入房间");
        return ;
    }
    if(ui->pb_openVideo->text() == "打开视频")
    {
        ui->pb_openVideo->setText("关闭视频");
        emit SIG_openVideo();
    }else
    {
        ui->pb_openVideo->setText("打开视频");
        emit SIG_closeVideo();
    }
}
//打开音频
void RoomDialog::on_pb_openAudio_clicked()
{
    if(m_roomid == 0)
    {
        QMessageBox::about(this,"提示","请加入房间");
        return ;
    }
    if(ui->pb_openAudio->text() == "打开音频")
    {
        ui->pb_openAudio->setText("关闭音频");
        emit SIG_openAudio();
    }else
    {
        ui->pb_openAudio->setText("打开音频");
        emit SIG_closeAudio();
    }
}

Ui::RoomDialog *RoomDialog::getUi() const
{
    return ui;
}
//设置房间id
void RoomDialog::setRoomid(const quint32 &roomid)
{
    m_roomid = roomid;
    ui->lb_title->setText(QString("房间号:%1").arg(m_roomid));
}
