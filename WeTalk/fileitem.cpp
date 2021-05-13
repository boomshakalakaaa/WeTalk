#include "fileitem.h"
#include "ui_fileitem.h"

FileItem::FileItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileItem)
{
    ui->setupUi(this);
    ui->pb_recv->hide();
    ui->pb_send->hide();
    ui->bar_uploadProgress->setValue(0);
}

FileItem::~FileItem()
{
    delete ui;
}
void FileItem::updateUploadProcess(int pos ,int max)
{
    ui->bar_uploadProgress->setMaximum(max);
    ui->bar_uploadProgress->setValue(pos);
}

void FileItem::setInfo(QString name,int nSize,bool show)
{
    ui->lb_filename->setText(name);
    QString size = QString::number(nSize);
    ui->lb_filesize->setText(size);
    if(show){
       ui->pb_send->show();
    }else{
       ui->pb_recv->show();
    }


}

Ui::FileItem *FileItem::getUi() const
{
    return ui;
}

