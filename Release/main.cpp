#include "mainwindow.h"
#include <QApplication>
#include <QStringList>
#include "Icon_Cache.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
