#include "qtsprat.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qtsprat mw;
    mw.show();

    return a.exec();
}
