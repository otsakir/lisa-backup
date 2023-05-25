#include "mainwindow.h"


#include <QApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QDebug>
#include <QSettings>
#include "task.h"
#include "terminal.h"
#include "scripting.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("otsakir");
    QApplication::setApplicationName("Lisa Backup");

    QSettings settings;
    //settings.setValue("initialized", false);  // remove settings file and uncomment this to start afresh

    if ( ! settings.value("initialized", false).toBool())
    {
        qDebug() << "Blank settings found. They will get initialized.";
        settings.setValue("TaskRunner", "internal" );
        settings.setValue("GenerateBashScripts", 0);
        settings.setValue("ShowConfirmation", 2); // i.e. true
        settings.setValue("initialized",true);

    }

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
        QString taskName = parser.value(runOption);
        qDebug().noquote() << QString("running backup task '%1'...").arg(taskName);

        BackupModel persisted;
        if (Tasks::loadTask(taskName,persisted)) {

            if (settings.value("ShowConfirmation").toInt() == 2)
            {
                QMessageBox messageBox(QMessageBox::Warning, "Starting backup task", QString("Backup task '%1' will be run. Do you agree ?").arg(taskName), QMessageBox::Yes | QMessageBox::Cancel, nullptr,Qt::Dialog);
                int ret = messageBox.exec();
                if (ret == QMessageBox::Yes) {
                    QVector<QString> commands;
                    Scripting::buildBackupCommands(persisted, commands);
                    for (QString& command: commands)
                    {
                        qDebug() << command;
                        QString out = Terminal::runShellCommand(command);
                        qDebug() << out;
                    }
                }
            }
        } else
        {
            qDebug().noquote() << QString("error loading task '%1'...").arg(taskName);
        }
        exit(0);
    }

    // "GUI" mode
    QIcon::setThemeName("Papirus");
    MainWindow w;
    w.show();

    return a.exec();
}
