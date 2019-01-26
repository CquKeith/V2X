#include "mainwindow.h"
#include <QApplication>
#include "appinit.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(":/qss/blue.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        qApp->setStyleSheet(qss);
        file.close();
    }

    //设置鼠标可以拖动
    AppInit::Instance()->start();

    MainWindow w;
    w.show();

    return a.exec();
}
