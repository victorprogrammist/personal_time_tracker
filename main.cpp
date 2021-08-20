#include "mainwindow.h"

#include <QApplication>

//#include <X11/Xlib.h>
//#include <X11/Xutil.h>

MainWindow* g_mainWindow = nullptr;

int main(int argc, char *argv[])
{

//    auto *display = XOpenDisplay(0);
//    XCloseDisplay(display);

    QApplication a(argc, argv);
    MainWindow w;
    g_mainWindow = &w;
    w.show();
    return a.exec();
}
