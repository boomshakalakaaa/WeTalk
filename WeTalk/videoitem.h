#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QWidget>
#include <QImage>
#include <QTime>
#include <QTimer>

namespace Ui {
class VideoItem;
}

class VideoItem : public QWidget
{
    Q_OBJECT

public:
    explicit VideoItem(QWidget *parent = 0);
    ~VideoItem();

    void paintEvent(QPaintEvent *event);            //在setImage中调用repaint就会走到paintEvent
    void mousePressEvent(QMouseEvent *event);       //鼠标点击事件
    void mouseDoubleClickEvent(QMouseEvent *event); //双击事件
    void setImage(const QImage &value);             //设置传输图片
    void setInfo(const QString &name,quint32 id);   //设置ui界面的用户信息

signals:
    void SIG_itemClicked();                         //发送点击信号

public slots:
    void slot_check();

private:
    Ui::VideoItem *ui;
public:
    QImage m_image;     //图片
    QTime m_lastTime;   //上一次刷新时间
    QTimer* m_Timer;    //定时器
    quint32 m_id;       //用户id
    QString m_name;     //用户名

};

#endif // VIDEOITEM_H
