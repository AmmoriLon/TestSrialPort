#include "widget.h"

#include <QApplication>
#include <QShowEvent>
#include <QDebug>
#include <unistd.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QCoreApplication a(argc, argv);

    Widget w;
    w.show();
    return a.exec();
}
