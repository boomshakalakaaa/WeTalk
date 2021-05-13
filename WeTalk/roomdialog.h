#ifndef ROOMDIALOG_H
#define ROOMDIALOG_H

#include <QDialog>

namespace Ui {
class RoomDialog;
}

class RoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoomDialog(QWidget *parent = 0);
    ~RoomDialog();

    void setRoomid(const quint32 &roomid);

    Ui::RoomDialog *getUi() const;

signals:
    void SIG_quitRoom(int id);
    void SIG_openAudio();
    void SIG_openVideo();
    void SIG_closeAudio();
    void SIG_closeVideo();

private slots:
    void on_pb_min_clicked();

    void on_pb_close_clicked();

    void on_pb_quitRoom_clicked();

    void clear();

    void on_pb_openVideo_clicked();

    void on_pb_openAudio_clicked();

private:
    Ui::RoomDialog *ui;
    quint32 m_roomid;   //房间id

};

#endif // ROOMDIALOG_H
