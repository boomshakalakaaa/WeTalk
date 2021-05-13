#include "videoitem.h"
#include "ui_videoitem.h"
#include <QPainter>

VideoItem::VideoItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoItem)
{
    ui->setupUi(this);
    m_lastTime = QTime::currentTime();  //获取当前时间
    m_Timer = new QTimer(this);         //初始化定时器，由当前类自己回收
    connect(m_Timer,SIGNAL(timeout()),this,SLOT(slot_check()));
    m_Timer->start(1000);
}

VideoItem::~VideoItem()
{
    m_Timer->stop();
    delete ui;
}
//定时检查是否停止刷新
void VideoItem::slot_check()
{
    //上一次刷新时间 距离 当前时间大于3秒
    if(m_lastTime.secsTo(QTime::currentTime()) > 3)
    {
        //填充一张黑色图片
        m_image.fill(Qt::black);
        //更新时间
        m_lastTime = QTime::currentTime();
        //绘图
        this->repaint();
    }
}
//在ui界面设置用户信息
void VideoItem::setInfo(const QString &name,quint32 id)
{
    m_name = name;
    m_id = id;
    ui->lb_name->setText(QString("用户名:%1").arg(m_name));
}

//绘图事件
void VideoItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //设置黑画刷，画黑背景
    painter.setBrush(Qt::black);
    painter.drawRect(0,0,this->width(),this->height());
    //
    if(m_image.size().width() <= 0)return;
    //等比例缩放
    QImage img = m_image.scaled(this->size(),Qt::KeepAspectRatio);
    //计算坐标
    int x = this->width() - img.width();
    int y = this->height() - this->ui->lb_name->height() - img.height();
    x = x/2;
    y = y/2;
    //贴图，在什么位置，贴什么图片
    painter.drawImage(QPoint(x,y),img);
}
//鼠标点击，发送信号
void VideoItem::mousePressEvent(QMouseEvent *event)
{
    //emit SIG_itemClicked(m_id);
}
void VideoItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit SIG_itemClicked();
}

void VideoItem::setImage(const QImage &value)
{
    m_image = value;
    //更新时间
    m_lastTime = QTime::currentTime();
    this->repaint();
}
