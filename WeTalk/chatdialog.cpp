#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QTime>
#include <QFileDialog>
#include <QFile>

ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog)
{
    ui->setupUi(this);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}
void ChatDialog::setInfo(int id,QString name,QString icon)
{
    m_id = id;
    m_name = name;
    m_icon = icon;
    ui->lb_title->setText(QString(m_name));

}

Ui::ChatDialog *ChatDialog::getUi() const
{
    return ui;
}

//发送消息
void ChatDialog::on_pb_send_clicked()
{
    //ui->tb_chat 浏览控件
    //ui->te_edit 编辑控件
    //获取编辑框发送的文本
    QString content = ui->te_edit->toPlainText();
    if(!content.isEmpty())
    {
        //在浏览控件上换行插入
        ui->tb_chat->append(QString("【我】%1").arg(QTime::currentTime().toString("hh:mm:ss")));
        ui->tb_chat->append(content);
        ui->tb_chat->append("");
        //清空编辑框文本
        ui->te_edit->clear();

        emit SIG_SendChatContent(m_id,content);
    }else
        return;
}

//发送文件
void ChatDialog::on_pb_sendfile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,"打开","D:/",
                                                "所有文件 (*.*);;");

    if(!path.isEmpty())
    {
        emit SIG_addToFileList(m_id,path);
    }
    else
        return ;
}
