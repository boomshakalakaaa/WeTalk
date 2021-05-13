#ifndef ADDFRIEND_H
#define ADDFRIEND_H

#include <QWidget>

namespace Ui {
class AddFriend;
}

class AddFriend : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriend(QWidget *parent = 0);
    ~AddFriend();
    void clear();
signals:
    void SIG_AddFriend(QString name);

private slots:
    //点击添加按钮
    void on_pb_add_clicked();

private:

    Ui::AddFriend *ui;
};

#endif // ADDFRIEND_H
