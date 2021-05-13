#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QRegExp>

LoginDialog::LoginDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
}

LoginDialog::~LoginDialog()
{

    delete ui;
}
//清空
void LoginDialog::on_pb_clear_clicked()
{
    slot_pb_register_clear();
}
//登录清空
void LoginDialog::slot_pb_login_clear()
{
    ui->le_name->setText("");
    ui->le_password->setText("");
}
//注册清空
void LoginDialog::slot_pb_register_clear()
{
    ui->le_register_name->setText("");
    ui->le_register_password->setText("");
    ui->le_register_confirm_password->setText("");
}
//注册
void LoginDialog::on_pb_register_clicked()
{
    QString name = ui->le_register_name->text();                //获取控件上的用户名
    QString password = ui->le_register_password->text();        //密码
    QString confirm = ui->le_register_confirm_password->text(); //确认密码
    //判断用户名、密码、确认密码是否为空
    if(name.isEmpty() || name.remove(" ").isEmpty()|| password.isEmpty() || confirm.isEmpty())
    {
        QMessageBox::information(this,"提示","用户名、密码或确认密码不能为空");
        return ;
    }
    //正则表达式匹配 用户名a-z A-Z 0-9出现1-10次
    QRegExp reg("[a-zA-Z0-9]{1,10}");
    bool res = reg.exactMatch(name);
    if(!res)
    {
        QMessageBox::information(this,"提示","用户名非法，要使用字母或数字");
        return ;
    }
    //判断用户名、密码长度是否符合要求，用户名长度不能超过10，密码长度不能超过20
    if(name.length() > 10 || password.length() > 20)
    {
        QMessageBox::information(this,"提示","用户名或密码过长");
        return ;
    }
    //判断密码和确认密码是否相等
    if(password != confirm)
    {
        QMessageBox::information(this,"提示","两次密码输入不一致");
        return ;
    }
    //发送信号
    emit SIG_registerCommit(name,password);
}
//登录
void LoginDialog::on_pb_login_clicked()
{
    QString name = ui->le_name->text();                //获取控件上的用户名
    QString password = ui->le_password->text();        //密码

    //判断用户名、密码、确认密码是否为空
    if(name.isEmpty() || name.remove(" ").isEmpty()|| password.isEmpty())
    {
        QMessageBox::information(this,"提示","用户名或密码不能为空");
        return ;
    }
    //正则表达式匹配 用户名a-z A-Z 0-9出现1-10次
    QRegExp reg("[a-zA-Z0-9]{1,10}");
    bool res = reg.exactMatch(name);
    if(!res)
    {
        QMessageBox::information(this,"提示","用户名非法，要使用字母或数字");
        return ;
    }
    //判断用户名、密码长度是否符合要求，用户名长度不能超过10，密码长度不能超过20
    if(name.length() > 10 || password.length() > 20)
    {
        QMessageBox::information(this,"提示","用户名或密码过长");
        return ;
    }
    emit SIG_loginCommit(name,password);
}
