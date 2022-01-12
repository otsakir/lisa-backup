#include "mainwindow.h"


#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    QObject::connect(ui->actionE_xit, &QAction::triggered,  )
//            connect(quitButton, &QPushButton::clicked, &app, &QCoreApplication::quit, Qt::QueuedConnection);


    //QObject::connect(w, &MainWindow::PleaseQuit, &a, QCoreApplication::quit, Qt::QueuedConnection);


    return a.exec();
}
