#include "fuzzymainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FuzzyMainWindow w;
    w.show();
    
    return a.exec();
}
