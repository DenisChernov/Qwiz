#include "mainwindow.h"
#include <QApplication>
#include <qtextcodec.h>
#include <QtPlugin>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
	a.addLibraryPath(a.applicationDirPath() + "/platforms");
    MainWindow w;
    w.showFullScreen();

    return a.exec();
}
