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
        settings.setValue("taskrunner/mode", "internal" );
        settings.setValue("taskrunner/GenerateBashScripts", 0);
        settings.setValue("taskrunner/ShowConfirmation", 2); // i.e. true
        settings.setValue("initialized",true);
    }
    settings.setValue("ApplicationFilePath", QApplication::applicationFilePath());

    //parse command line
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption runOption(QStringList() << "r" << "run", "Run the specified task", "task name");
    parser.addOption(runOption);
    parser.process(a);

    if (parser.isSet(runOption))    // "CLI" mode
    {
        QString taskName = parser.value(runOption);
        qDebug().noquote() << QString("running backup task '%1'...").arg(taskName);

        BackupModel persisted;
        if (Tasks::loadTask(taskName,persisted)) {

            if (settings.value("taskrunner/ShowConfirmation").toInt() == 2)
            {
                QMessageBox messageBox(QMessageBox::Information, "Lisa Backup", QString("Backup task '%1' triggered. Shall i proceed ?").arg(taskName), QMessageBox::Yes | QMessageBox::Cancel, nullptr,Qt::Dialog);
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
