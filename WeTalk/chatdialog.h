#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "wetalk.h"
#include "fileitem.h"


namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = 0);
    ~ChatDialog();

    void setInfo(int id,QString name,QString icon = QString(":/tx/8.png"));
signals:
    void SIG_SendChatContent(int id,QString content);
    void SIG_SendFile(int id);
    void SIG_addToFileList(int id,QString path);

private:
    Ui::ChatDialog *ui;
public:
    int m_id;
    QString m_name;
    QString m_icon;
    Ui::ChatDialog *getUi() const;


private slots:
    void on_pb_send_clicked();
    void on_pb_sendfile_clicked();
};

#endif // CHATDIALOG_H
