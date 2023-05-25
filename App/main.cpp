#include "mainwindow.h"


#include <QApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QDebug>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //parse command line
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption runOption(QStringList() << "r" << "run", "Run the specified task", "task name");
    parser.addOption(runOption);
    QCommandLineOption confirmOption(QStringList() << "c" << "confirm", "Show confirmation dialog");
    parser.addOption(confirmOption);

    parser.process(a);

    if (parser.isSet(confirmOption) && !parser.isSet(runOption))
    {
        fputs(qPrintable("error: can't show confirmation in GUI mode. Maybe run with '-r' ?"), stderr);
        fputs("\n\n", stderr);
        fputs(qPrintable(parser.helpText()), stderr);
        return 1;
    }

    if (parser.isSet(runOption))    // "CLI" mode
    {
        qDebug().noquote() << QString("running backup task '%1'...").arg(parser.value(runOption));
        exit(0);
    }

    QApplication::setOrganizationName("otsakir");
    QApplication::setApplicationName("Lisa Backup");

    QSettings settings;

    //settings.setValue("initialized", false);

    if ( ! settings.value("initialized", false).toBool())
    {
        qDebug() << "Blank settings found. They will get initialized.";
        settings.setValue("TaskRunner", "internal" );
        settings.setValue("GenerateBashScripts", 0);
        settings.setValue("initialized",true);

    }


    // "GUI" mode
    QIcon::setThemeName("Papirus");
    MainWindow w;
    w.show();

    return a.exec();
}
