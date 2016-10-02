#include "whats_happening.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    _Whats_Happening w;
    w.show();

    return a.exec();
}
