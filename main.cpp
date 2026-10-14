#include "welcomewindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft YaHei"));

    WelcomeWindow w;

    w.show();

    return a.exec();
}
