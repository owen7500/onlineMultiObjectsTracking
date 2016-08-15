#include "targetTrackingUI.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::addLibraryPath("./plugins");
    TargetTracking w;
    w.show();

    return a.exec();
}
