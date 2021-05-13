#ifndef FILEITEM_H
#define FILEITEM_H

#include <QWidget>

namespace Ui {
class FileItem;
}

class FileItem : public QWidget
{
    Q_OBJECT

public:
    explicit FileItem(QWidget *parent = 0);
    ~FileItem();
public:
    void updateUploadProcess(int pos ,int max);
    void setInfo(QString name,int nSize,bool show);

    Ui::FileItem *getUi() const;

private:
    Ui::FileItem *ui;
};

#endif // FILEITEM_H
