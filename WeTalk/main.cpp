#include "wetalk.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WeTalk w;
    w.hide();

    return a.exec();
}
