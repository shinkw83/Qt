#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QFont>
#include <QSharedMemory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QSharedMemory shared("FarmLogAgent");
    if (!shared.create(512, QSharedMemory::ReadWrite)) {
        QMessageBox::information(&w, "Error", QString::fromLocal8Bit("프로그램이 이미 실행 되고 있습니다."));
        exit(0);
    }
    w.show();
    return a.exec();
}