#include "skeletonApp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    skeletonApp w;
    w.show();
    return a.exec();
}
