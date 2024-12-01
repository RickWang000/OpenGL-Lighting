#include "QtOpenGLLighting.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtOpenGLLighting w;
    w.show();
    return a.exec();
}
