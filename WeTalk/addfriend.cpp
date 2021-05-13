#include "addfriend.h"
#include "ui_addfriend.h"

AddFriend::AddFriend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);
}

AddFriend::~AddFriend()
{
    delete ui;
}
//点击添加按钮
void AddFriend::on_pb_add_clicked()
{
    emit SIG_AddFriend(ui->le_friendName->text());
}
//每次打开对话框都清空
void AddFriend::clear()
{
    ui->le_friendName->setText("");
}
